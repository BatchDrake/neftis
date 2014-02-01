/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) <year>  <name of author>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
 
#include <types.h>

#include <mm/regions.h>
#include <mm/coloring.h>
#include <mm/vm.h>
#include <mm/spalloc.h>

#include <misc/list.h>

#include <arch.h>
#include <kctx.h>

INLINE struct vanon_strip *
vanon_strip_new (void)
{
  CONSTRUCTOR_BODY_OF_STRUCT (vanon_strip);
}

INLINE void
vanon_strip_destroy (struct vanon_strip *strip)
{
  if (strip->vs_pages)
    page_free (strip->vs_phys_start, strip->vs_pages);
  
  spfree (strip);
}

INLINE void
vanon_strip_destroy_nofree (struct vanon_strip *strip)
{
  spfree (strip);
}

INLINE void
vm_region_anon_destroy (struct vm_region *region)
{
  struct vanon_strip *strip, *next;
  
  ASSERT (region->vr_type == VREGION_TYPE_ANON        ||
          region->vr_type == VREGION_TYPE_ANON_NOSWAP ||
          region->vr_type == VREGION_TYPE_STACK       ||
          region->vr_type == VREGION_TYPE_IOMAP);
          
  strip = region->vr_strips.anon;
  
  while (strip)
  {
    next = LIST_NEXT (strip);
    
    if (region->vr_type == VREGION_TYPE_IOMAP)
      vanon_strip_destroy_nofree (strip);
    else
      vanon_strip_destroy (strip);
      
    strip = next;
  }
}

struct vm_region *
vm_region_new (int type)
{
  struct vm_region *new;
  
  if (type != VREGION_TYPE_KERNEL &&
      type != VREGION_TYPE_ANON   &&
      type != VREGION_TYPE_ANON_NOSWAP &&
      type != VREGION_TYPE_IOMAP)
  {
    debug ("region type %d not supported by microkernel\n", type);
    return NULL;
  }
  
  CONSTRUCT_STRUCT (vm_region, new);
  
  new->vr_type = type;
  
  return new;
}

void
vm_region_destroy (struct vm_region *region)
{
  switch (region->vr_type)
  {
    case VREGION_TYPE_KERNEL:
      break;
      
    case VREGION_TYPE_ANON:
    case VREGION_TYPE_ANON_NOSWAP:
    case VREGION_TYPE_STACK:
    case VREGION_TYPE_IOMAP:
      vm_region_anon_destroy (region);
      break;
      
    default:
      FAIL ("corrupted vm region struct!\n");
  }
  
  spfree (region);
}

struct vm_space *
vm_space_new (void)
{
  CONSTRUCTOR_BODY_OF_STRUCT (vm_space);
}

INLINE struct vanon_strip *
vm_get_colored_range (struct mm_region *from, int color, int up_to)
{
  struct vanon_strip *new;
  busword_t page;
  int i;
  
  /* If I do spin_lock here, deadlock (you see, spalloc can call
     to page_alloc, that locks mem regions */
     
  if ((new = vanon_strip_new ()) == NULL)
    return NULL;
  
  spin_lock (&from->mr_lock);
  
  new->vs_pages = 0;
  
  for (i = 0; i < up_to; i++)
  {
    if ((page = mm_get_colored_page (from, color + i)) == 
      (busword_t) KERNEL_ERROR_VALUE)
      break;

    if (!new->vs_pages)
    {
      MMR_MARK_PAGE (from, page);
      new->vs_phys_start = (busword_t) MMR_PAGE_TO_ADDR (from, page);
      new->vs_pages      = 1;
      new->vs_ref_cntr   = 1;
    }
    else if (page == 
      MMR_ADDR_TO_PAGE (from, new->vs_phys_start) + new->vs_pages)
    {
      MMR_MARK_PAGE (from, page);
      new->vs_pages++;
    }
    else
      break;
  }
  
  spin_unlock (&from->mr_lock);

  if (!new->vs_pages)
  {
    vanon_strip_destroy (new);
    new = NULL;
  }
  
  return new;
}

struct vanon_strip *
vm_alloc_colored (struct mm_region *region, busword_t virtual, int number)
{
  struct vanon_strip *list, *new;
  busword_t page_count;
  
  int order = 0;
  
  int color;
  
  list = NULL;
    
  color = ADDR_COLOR (mm_get_cache_size (), virtual);
  
  page_count = 0;

  for (; page_count < number; page_count += new->vs_pages)
  {
    if (PTR_UNLIKELY_TO_FAIL 
      (
        new = vm_get_colored_range (region, color, number - page_count)
      )
    )
    {
      if (list != NULL)
      {        
        vanon_strip_destroy (list);
        return KERNEL_INVALID_POINTER;
      }
    }
    
    new->vs_virt_start = virtual + (page_count << __PAGE_BITS);
    
    sorted_list_insert ((void **) &list, new, new->vs_virt_start);
  }
  
  return list;
}

void
vm_space_destroy (struct vm_space *space)
{
  struct vm_region *region, *next;
  
  region = space->vs_regions;
  
  while (region)
  {
    next = LIST_NEXT (region);
    
    vm_region_destroy (region);
    
    region = next;
  }
  
  __vm_free_page_table (space->vs_pagetable);
}

/* Next level */

INLINE int
vm_test_range (struct vm_space *space, busword_t start, busword_t pages)
{
  struct vm_region *region_prev, *region_next;
  
  region_prev = 
    sorted_list_get_previous ((void **) &space->vs_regions, start);
    
  region_next = 
    sorted_list_get_next ((void **) &space->vs_regions, start);
    
  if (region_prev != NULL)
    if (region_prev->vr_virt_end >= start)
      return 0;
    
  if (region_next != NULL)
    if (region_next->vr_virt_start < (start + pages * PAGE_SIZE - 1))
      return 0;
    
  
  return 1;
}

void
vm_region_invalidate (struct vm_region *region)
{
  struct vanon_strip *strip;
  int i;
  
  strip = region->vr_strips.anon;

  while (strip != NULL)
  {
    __vm_flush_pages (strip->vs_virt_start, strip->vs_pages);
    
    strip = LIST_NEXT (strip);
  }
}

int
vm_space_add_region (struct vm_space *space, struct vm_region *region)
{
  if (!vm_test_range (space, 
                      region->vr_virt_start,
                      (region->vr_virt_end + 1 - region->vr_virt_start) /
                        PAGE_SIZE))
    return KERNEL_ERROR_VALUE;
  
  sorted_list_insert ((void *) &space->vs_regions,
                      region,
                      region->vr_virt_start);


  vm_update_region (space, region);
  
  return KERNEL_SUCCESS_VALUE;
}

int
vm_space_overlap_region (struct vm_space *space, struct vm_region *region)
{
  sorted_list_insert ((void *) &space->vs_regions,
                      region,
                      region->vr_virt_start);
                      
  return 0;
}

INLINE void
vanon_strips_set_owner (struct vanon_strip *strips, struct vm_region *region)
{
  while (strips != NULL)
  {
    strips->vs_region = region;
    strips = LIST_NEXT (strips);
  }
}

struct vm_region *
vm_region_anonmap (busword_t virt, busword_t pages)
{
  int i;
  struct vm_region *new;
  struct mm_region *region;
  struct vanon_strip *strips, *this;
  
  extern struct mm_region *mm_regions;
  
  /* TODO: do this NUMA-friendly */

  PTR_RETURN_ON_PTR_FAILURE (new = vm_region_new (VREGION_TYPE_ANON));
                         
  region = mm_regions;

  while (region != NULL)
  {
    if ((strips = vm_alloc_colored (region, virt, pages)) != NULL)
      break;
      
    region = region->mr_next;
  }
  
  if (PTR_UNLIKELY_TO_FAIL (strips))
  { 
    vm_region_destroy (new);
    return KERNEL_INVALID_POINTER;
  }
  
  vanon_strips_set_owner (strips, new);

  new->vr_access      = VM_PAGE_READABLE;
  new->vr_virt_start  = virt;
  new->vr_virt_end    = virt + (pages << __PAGE_BITS) - 1;
  new->vr_strips.anon = strips;
  
  return new;
}

/* User for allocating stack regions. This is basically an anonmap
   that starts at stack_bottom - (pages << __PAGE_BITS) and ends
   at stack_bottom - 1. This means that if we want a stack bottom at
   0xbfffffff (Linux-x86 style), we need to call:

   vm_region_stack (0xc0000000, 16)

   Which, under x86, allocates 64 KiB of stack right below of the
   kernel space
  */

struct vm_region *
vm_region_stack (busword_t stack_bottom, busword_t pages)
{
  busword_t top_page = stack_bottom - (pages << __PAGE_BITS);
  struct vm_region *new;

  PTR_RETURN_ON_PTR_FAILURE (new = vm_region_anonmap (top_page, pages));

  new->vr_type   = VREGION_TYPE_STACK;
  new->vr_access = VREGION_ACCESS_READ | VREGION_ACCESS_WRITE;

  return new;
}

/* TODO: design macros to generalize dynamic allocation */
struct vm_region *
vm_region_iomap (busword_t virt, busword_t phys, busword_t pages)
{
  struct vm_region *region;
  struct vanon_strip *strip;
  
  PTR_RETURN_ON_PTR_FAILURE (region = vm_region_new (VREGION_TYPE_IOMAP));
    
  if (PTR_UNLIKELY_TO_FAIL (strip = vanon_strip_new ()))
  {
    spfree (region);
    return KERNEL_INVALID_POINTER;
  }
  
  strip->vs_ref_cntr     = 1;
  strip->vs_pages        = pages;
  strip->vs_virt_start   = virt;
  strip->vs_phys_start   = phys;
  strip->vs_region       = region;
  
  region->vr_virt_start  = virt;
  region->vr_virt_end    = virt + (pages << __PAGE_BITS) - 1;
  region->vr_strips.anon = strip;
  
  return region;
}

struct vm_region *
vm_region_shared (busword_t virt, busword_t phys, busword_t pages)
{
  struct vm_region *region;
  
  PTR_RETURN_ON_PTR_FAILURE (region = vm_region_new (VREGION_TYPE_KERNEL));
  
  region->vr_virt_start  = virt;
  region->vr_virt_end    = virt + (pages << __PAGE_BITS) - 1;
  region->vr_phys_start  = phys;
  
  return region;
}

busword_t
virt2phys (const struct vm_space *space, busword_t virt)
{
  struct vm_region *curr;
  struct vanon_strip *strip;
  
  curr = space->vs_regions;

  while (curr)
  {
    if (virt >= curr->vr_virt_start && virt <= curr->vr_virt_end)
    {
      if (curr->vr_type == VREGION_TYPE_KERNEL ||
          curr->vr_type == VREGION_TYPE_IOMAP)
        return virt - curr->vr_virt_start + curr->vr_phys_start;
      
      strip = curr->vr_strips.anon;

      while (strip)
      {
        if (virt >= strip->vs_virt_start && ((virt - strip->vs_virt_start) >> __PAGE_BITS) < strip->vs_pages)
          return virt - strip->vs_virt_start + strip->vs_phys_start;
        
        strip = LIST_NEXT (strip);
      }

      return 0;
    }
    else if (virt < curr->vr_virt_start) /* As it's sorted.. */
      return 0;
    
    curr = LIST_NEXT (curr);
  }

  return 0;
}


int
copy2virt (const struct vm_space *space, busword_t virt, const void *orig, busword_t size)
{
  busword_t offset = virt &  PAGE_MASK; 
  busword_t page   = virt & ~PAGE_MASK;
  busword_t phys, len;
  
  /* TODO: implement this faster */

  while (size)
  {
    if (!(phys = virt2phys (space, page)))
      break;
    
    if ((len = PAGE_SIZE - offset) > size)
      len = size;

    memcpy (phys + offset, orig, len);

    orig += len;
    page += PAGE_SIZE;
    size -= len;
    
    offset = 0;
  }

  return size;
}

INLINE int
vm_update_tables_anon (struct vm_space *space, struct vm_region *region)
{
  BYTE flags;
  struct vanon_strip *strip;
  
  ASSERT (region->vr_type == VREGION_TYPE_ANON        ||
          region->vr_type == VREGION_TYPE_ANON_NOSWAP ||
          region->vr_type == VREGION_TYPE_STACK);
          
  flags = region->vr_access & 0xe; /* 1110 */
  
  strip = region->vr_strips.anon;
  
  while (strip)
  {
    RETURN_ON_FAILURE
    (__vm_map_to (space->vs_pagetable, 
                     strip->vs_virt_start,
                     strip->vs_virt_start,
                     strip->vs_pages,
                     flags)
    );
      
    strip = LIST_NEXT (strip);
  }
  
  return KERNEL_SUCCESS_VALUE;
}


INLINE int
vm_update_tables_iomap (struct vm_space *space, struct vm_region *region)
{
  BYTE flags;
  struct vanon_strip *strip;
  
  ASSERT (region->vr_type == VREGION_TYPE_IOMAP);
  ASSERT (region->vr_strips.anon != NULL);
  
  strip = region->vr_strips.anon;
  
  flags = (region->vr_access & 0xe) | VM_PAGE_IOMAP;

  RETURN_ON_FAILURE
  (__vm_map_to (space->vs_pagetable, 
                     strip->vs_virt_start,
                     strip->vs_virt_start, /* There's identity paging when dealing with iomap tables */
                     strip->vs_pages,
                     flags)
  );
  
  return KERNEL_SUCCESS_VALUE;
}

INLINE int
vm_update_tables_kernel (struct vm_space *space, struct vm_region *region)
{
  BYTE flags;
  
  ASSERT (region->vr_type == VREGION_TYPE_KERNEL);
  flags = (region->vr_access & 0xe) | VM_PAGE_KERNEL;

  RETURN_ON_FAILURE
  (__vm_map_to (space->vs_pagetable, 
                     region->vr_virt_start,
                     region->vr_phys_start,
                     (region->vr_virt_end - region->vr_virt_start + 1) >>
                       __PAGE_BITS,
                     flags)
  );
    
  return KERNEL_SUCCESS_VALUE;
}

int
vm_update_region (struct vm_space *space, struct vm_region *region)
{
  if (space->vs_pagetable == NULL)
    space->vs_pagetable = __vm_alloc_page_table ();

  switch (region->vr_type)
  {
    case VREGION_TYPE_ANON:
    case VREGION_TYPE_ANON_NOSWAP:
    case VREGION_TYPE_STACK:
      RETURN_ON_FAILURE (vm_update_tables_anon (space, region));
      break;
        
    case VREGION_TYPE_KERNEL:
      RETURN_ON_FAILURE (vm_update_tables_kernel (space, region));
      break;
        
    case VREGION_TYPE_IOMAP:
      RETURN_ON_FAILURE (vm_update_tables_iomap (space, region));
      break;
        
    default:
      FAIL ("unknown region type %d", region->vr_type);
      break;
  }

  return KERNEL_SUCCESS_VALUE;
}

/* Transforms segment information into hardware translation data (i.e.
   update page tables, etc) */
int
vm_update_tables (struct vm_space *space)
{
  struct vm_region *this;
  int ret;
  
  this = space->vs_regions;
  
  while (this)
  {
    
    if ((ret = vm_update_region (space, this)) != KERNEL_SUCCESS_VALUE)
      return ret;
    
    this = LIST_NEXT (this);
  }
  
  return KERNEL_SUCCESS_VALUE;
}

struct vm_space *
vm_kernel_space (void)
{
  extern struct mm_region *mm_regions;
  
  struct vm_space  *new_space;
  struct vm_region *new_region;
  struct mm_region *this;
  
  PTR_RETURN_ON_PTR_FAILURE (new_space = vm_space_new ());

  this = mm_regions;
  
  while (this != NULL)
  {
    if (PTR_UNLIKELY_TO_FAIL (new_region = vm_region_new (VREGION_TYPE_KERNEL)))
    {
      error ("Failed to register VM kernel space\n");
      
      vm_space_destroy (new_space);
      return KERNEL_INVALID_POINTER;
    }

    new_region->vr_virt_start = (busword_t) this->mr_start;
    new_region->vr_phys_start = (busword_t) this->mr_start;
    new_region->vr_virt_end   = (busword_t) this->mr_end;
    new_region->vr_access     = VREGION_ACCESS_READ  |
                                VREGION_ACCESS_WRITE |
                                VREGION_ACCESS_EXEC;
                                
    if (UNLIKELY_TO_FAIL (vm_space_add_region (new_space, new_region)))
    {
      error ("Failed to add region to VM kernel space\n");
      vm_region_destroy (new_region);
      vm_space_destroy (new_space);
      
      return KERNEL_INVALID_POINTER;
    }
    
    this = this->mr_next;
  }
  
  if (UNLIKELY_TO_FAIL (vm_kernel_space_map_image (new_space)))
  {
    error ("couldn't map kernel image\n");
    
    vm_space_destroy (new_space);
    
    return KERNEL_INVALID_POINTER;
  }
  
  if (UNLIKELY_TO_FAIL (vm_kernel_space_map_io (new_space)))
  {
    error ("Cannot map IO!\n");

    vm_space_destroy (new_space);
    
    return KERNEL_INVALID_POINTER;
  }
  
  if (UNLIKELY_TO_FAIL (vm_update_tables (new_space)))
  {
    error ("failed to update tables");
    return KERNEL_INVALID_POINTER;
  }
  
  return new_space;
}

struct vm_space *
vm_bare_process_space (void)
{
  extern struct mm_region *mm_regions;
  busword_t stack_bottom;
  struct vm_space  *new_space;
  struct vm_region *stack;
  
  PTR_RETURN_ON_PTR_FAILURE (new_space = vm_space_new ());

  /* Map kernel space */
  
  if (UNLIKELY_TO_FAIL (vm_kernel_space_map_image (new_space)))
  {
    error ("couldn't map kernel image\n");
    
    vm_space_destroy (new_space);
    
    return KERNEL_INVALID_POINTER;
  }

  stack_bottom = vm_get_prefered_stack_bottom ();

  if (PTR_UNLIKELY_TO_FAIL (stack = vm_region_stack (stack_bottom, TASK_SYS_STACK_PAGES)))
  {
    error ("couldn't allocate stack (bottom = %p)\n", stack_bottom - 1);
    
    vm_space_destroy (new_space);

    return KERNEL_INVALID_POINTER;
  }

  if (UNLIKELY_TO_FAIL (vm_space_add_region (new_space, stack)))
  {
    error ("couldn't add stack to new space (bottom = %p)\n", stack_bottom - 1);

    vm_region_destroy (stack);
    
    vm_space_destroy (new_space);

    return KERNEL_INVALID_POINTER;
  }
  
  if (UNLIKELY_TO_FAIL (vm_update_tables (new_space)))
  {
    error ("failed to update tables");

    vm_space_destroy (new_space);
    
    return KERNEL_INVALID_POINTER;
  }
  
  return new_space;
}

const char *
vm_type_to_string (int type)
{
  char *types[] = {"anon  ", "noswap ", "stack ", "shared", "cow   ", "iomap ",
  "zero  ", "kernel", "custom"};
  
  if (type < 0 || type > 8)
    return "<unk> ";
  else
    return types[type];
}

void
vm_space_debug (struct vm_space *space)
{
  struct vm_region *this;
  
  this = space->vs_regions;
  
  while (this)
  {
    printk ("%y-%y: %s %H\n", this->vr_virt_start,
      this->vr_virt_end,
      vm_type_to_string (this->vr_type),
      this->vr_virt_end - this->vr_virt_start + 1);
      
    this = (struct vm_region *) LIST_NEXT (this);
  }
}

void
vm_init (void)
{ 
  /* TODO: fix for SMP */
  
  MANDATORY (SUCCESS_PTR (
    current_kctx->kc_vm_space = vm_kernel_space ()
    )
  );

  hw_vm_init ();
}

DEBUG_FUNC (vanon_strip_new);
DEBUG_FUNC (vanon_strip_destroy);
DEBUG_FUNC (vanon_strip_destroy_nofree);
DEBUG_FUNC (vm_update_tables_anon);
DEBUG_FUNC (vm_update_tables_iomap);
DEBUG_FUNC (vm_update_tables_kernel);
DEBUG_FUNC (vm_update_region);
DEBUG_FUNC (vm_update_tables);
DEBUG_FUNC (vm_region_anon_destroy);
DEBUG_FUNC (vm_region_new);
DEBUG_FUNC (vm_region_destroy);
DEBUG_FUNC (vm_space_new);
DEBUG_FUNC (vm_space_destroy);
DEBUG_FUNC (vm_test_range);
DEBUG_FUNC (vm_space_add_region);
DEBUG_FUNC (vm_region_iomap);
DEBUG_FUNC (vm_kernel_space);
DEBUG_FUNC (vm_kernel_space_map_image);
DEBUG_FUNC (vm_bare_process_space);
DEBUG_FUNC (vm_type_to_string);
DEBUG_FUNC (vm_space_debug);


