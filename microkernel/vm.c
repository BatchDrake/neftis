/*
 *    Virtual memory subsystem.
 *    Copyright (C) 2014  Gonzalo J. Carracedo
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

#include <asm/layout.h>

#include <mm/regions.h>
#include <mm/coloring.h>
#include <mm/vm.h>
#include <mm/anon.h>
#include <mm/salloc.h>
#include <mm/vremap.h>

#include <misc/list.h>
#include <misc/object.h>
#include <task/loader.h>

#include <string.h>
#include <arch.h>
#include <kctx.h>

struct vm_region *
vm_region_new (busword_t start, busword_t end, struct vm_region_ops *ops, void *data)
{
  struct vm_region *new;
  
  CONSTRUCT_STRUCT (vm_region, new);

  new->vr_ops = ops;
  new->vr_ops_data = data;
  new->vr_virt_start = start;
  new->vr_virt_end   = end;

  return new;
}

int
vm_region_set_desc (struct vm_region *region, const char *desc)
{
  char *new_desc;

  if ((new_desc = strdup (desc)) == NULL)
    return -1;
  
  if (region->vr_desc != NULL)
    sfree_irq (region->vr_desc);

  region->vr_desc = new_desc;

  return 0;
}

void
vm_region_destroy (struct vm_region *region, struct task *task)
{
  /* Check whether region can be destroyed. */
  if (region->vr_ops->destroy != NULL)
    if ((region->vr_ops->destroy) (task, region) == -1)
      return;

  if (region->vr_desc != NULL)
    sfree_irq (region->vr_desc);
  
  if (region->vr_type == VREGION_TYPE_PAGEMAP && region->vr_page_set != NULL)
    kernel_object_ref_close (region->vr_page_set);
  
  sfree (region);
}

int
vm_region_unmap_page (struct vm_region *region, busword_t virt)
{
  if (region->vr_type != VREGION_TYPE_RANGEMAP)
    return KERNEL_ERROR_VALUE;

  return vm_page_set_remove (REFCAST (struct vm_page_set, region->vr_page_set), virt - region->vr_virt_start);
}

int
vm_region_map_page (struct vm_region *region, busword_t virt, busword_t phys, DWORD flags)
{
  void **slot;

  if (region->vr_type == VREGION_TYPE_RANGEMAP)
    return KERNEL_ERROR_VALUE;

  return vm_page_set_put (REFCAST (struct vm_page_set, region->vr_page_set), virt - region->vr_virt_start, phys);
}

int
vm_region_map_pages (struct vm_region *region, busword_t virt, busword_t phys, DWORD flags, busword_t num)
{
  busword_t i;

  for (i = 0; i < num; ++i)
    if (FAILED (vm_region_map_page (region, virt + (i << __PAGE_BITS), phys + (i << __PAGE_BITS), flags)))
      return -1;

  return 0;
}

busword_t
vm_region_translate_page (struct vm_region *region, busword_t virt, DWORD *flags)
{
  busword_t addr;

  if (region->vr_unlinked_remap)
    return virt;
  
  if (region->vr_type == VREGION_TYPE_RANGEMAP)
  {
    if (virt < region->vr_virt_start || virt > region->vr_virt_end)
      return (busword_t) KERNEL_ERROR_VALUE;

    addr = virt - region->vr_virt_start + region->vr_phys_start;
  }
  else
  {
    if (vm_page_set_translate (REFCAST (struct vm_page_set, region->vr_page_set), virt - region->vr_virt_start, &addr) == KERNEL_ERROR_VALUE)
      return (busword_t) KERNEL_ERROR_VALUE;
  }
  
  return addr;
}

int
vm_region_resize (struct vm_region *region, busword_t start, busword_t pages)
{
  struct rbtree_node *prev_node, *next_node;
  struct vm_region   *prev, *next;
  
  start = PAGE_START (start);

  if (region->vr_ops->resize == NULL)
  {
    error ("region doesn't support resizing\n");
    return -1;
  }
  
  if (start + pages * PAGE_SIZE < start)
  {
    error ("region rollover\n");
    return -1;
  }
  
  ASSERT (region->vr_node != NULL);
			   
  if ((prev_node = rbtree_node_prev (region->vr_node)) != NULL)
  {
    prev = (struct vm_region *) rbtree_node_data (prev_node);

    if (start <= prev->vr_virt_end)
    {
      error ("refusing to move: overlapping regions\n");
      return -1;
    }
  }

  if ((next_node = rbtree_node_next (region->vr_node)) != NULL)
  {
    next = (struct vm_region *) rbtree_node_data (next_node);
    
    if (next->vr_virt_start < start + pages * PAGE_SIZE)
    {
      error ("refusing to grow: overlapping regions\n");
      return -1;
    }
  }
  
  if ((region->vr_ops->resize) (get_current_task (), region, start, pages) == -1)
    return -1;

  region->vr_virt_start = start;
  region->vr_virt_end   = start + pages * PAGE_SIZE - 1;

  return 0;
}

struct vm_space *
vm_space_new (void)
{
  struct vm_space *new;
  
  if ((new = salloc (sizeof (struct vm_space))) == NULL)
    return NULL;

  if ((new->vs_region_tree = rbtree_new ()) == NULL)
  {
    sfree (new);

    return NULL;
  }

  new->vs_pagetable = NULL;

  return new;
}

static void
__rbtree_vm_region_dtor (void *region, void *dtor_data)
{
  vm_region_destroy ((struct vm_region *) region, NULL);
}

void
vm_space_destroy (struct vm_space *space)
{
  struct vm_region *region, *next;

  rbtree_set_dtor (space->vs_region_tree, __rbtree_vm_region_dtor, NULL);

  rbtree_destroy (space->vs_region_tree);
  
  __vm_free_page_table (space->vs_pagetable);

  sfree (space);
}

/* Next level */

/* TODO: improve this using information in radix trees */
struct vm_region *
vm_space_find_first_in_range (struct vm_space *space, busword_t start, busword_t pages)
{
  struct rbtree_node *leftwards, *rightwards;
  struct vm_region *region_prev = NULL, *region_next = NULL;

  if ((leftwards = rbtree_search (space->vs_region_tree, start, RB_LEFTWARDS)) != NULL)
    region_prev = rbtree_node_data (leftwards);

  if ((rightwards = rbtree_search (space->vs_region_tree, start, RB_RIGHTWARDS)) != NULL)
    region_next = rbtree_node_data (rightwards);

  if (region_prev != NULL)
    if (start <= region_prev->vr_virt_end)
      return region_prev;
  
  if (region_next != NULL)
    if (region_next->vr_virt_start < (start + pages * PAGE_SIZE - 1))
      return region_next;
  
  return NULL;
}

int
vm_space_walk (struct vm_space *space, int (*callback) (struct vm_space *, struct vm_region *, void *), void *data)
{
  struct rbtree_node *this;
  int ret;
  
  this = rbtree_get_first (space->vs_region_tree);

  while (this != NULL)
  {
    if ((ret = (callback) (space, (struct vm_region *) rbtree_node_data (this), data)) != 0)
      return ret;

    this = rbtree_node_next (this);
  }

  return 0;
}

INLINE int
vm_test_range (struct vm_space *space, busword_t start, busword_t pages)
{
  struct vm_region *region;
  
  region = vm_space_find_first_in_range (space, start, pages);
  
  return region == NULL;
}

static int
__invalidate_pages (radixkey_t virt, void **phys, radixtag_t *perms, void *data)
{
  __vm_flush_pages ((busword_t) virt, 1);

  return KERNEL_SUCCESS_VALUE;
}

void
vm_region_invalidate (struct vm_region *region)
{
  if (region->vr_type == VREGION_TYPE_PAGEMAP)
    (void) vm_page_set_walk (REFCAST (struct vm_page_set, region->vr_page_set), __invalidate_pages, NULL);
}

int
vm_space_add_region (struct vm_space *space, struct vm_region *region)
{
  struct rbtree_node *node;
  
  if (!vm_test_range (space, 
                      region->vr_virt_start,
                      (region->vr_virt_end + 1 - region->vr_virt_start) /
                        PAGE_SIZE))
  {
    error ("Map region %p-%p: overlapping regions!\n", region->vr_virt_start, region->vr_virt_end);
    return KERNEL_ERROR_VALUE;
  }

  if (rbtree_insert_ex (space->vs_region_tree, region->vr_virt_start, region, &node) == -1)
  {
    error ("Out of memory while inserting region\n");
    return KERNEL_ERROR_VALUE;
  }

  region->vr_node = node;
  
  vm_update_region (space, region);
  
  return KERNEL_SUCCESS_VALUE;
}

int
vm_space_overlap_region (struct vm_space *space, struct vm_region *region)
{
  if (rbtree_insert (space->vs_region_tree, region->vr_virt_start, region) == -1)
  {
    error ("Out of memory while inserting region\n");
    return KERNEL_ERROR_VALUE;
  }

  vm_update_region (space, region);
  
  return 0;
}

struct vm_region *
vm_space_find_region_by_role (const struct vm_space *space, int role)
{
  struct vm_region *region;
  struct rbtree_node *node;

  node = rbtree_get_first (space->vs_region_tree);
  
  while (node != NULL)
  {
    region = (struct vm_region *) rbtree_node_data (node);

    if (region->vr_role == role)
      return region;

    node = rbtree_node_next (node);
  }

  return NULL;
}

struct vm_region *
vm_space_find_region (const struct vm_space *space, busword_t virt)
{
  struct vm_region *region;
  struct rbtree_node *node;
  
  if ((node = rbtree_search (space->vs_region_tree, virt, RB_LEFTWARDS)) != NULL)
  {
    region = (struct vm_region *) rbtree_node_data (node);

    if (region->vr_virt_start <= virt && virt <= region->vr_virt_end)
      return region;
  }

  return NULL;
}

busword_t
virt2phys (const struct vm_space *space, busword_t virt)
{
  struct vm_region *region;
  busword_t off  = virt &  PAGE_MASK;
  busword_t page = virt & ~PAGE_MASK;
  busword_t phys;

  if ((region = vm_space_find_region (space, virt)) != NULL)
    if ((phys = vm_region_translate_page (region, page, NULL)) != (busword_t) KERNEL_ERROR_VALUE)
      return phys | off;

  return 0;
}

/* Returns the number of transfered bytes */
int
copy2virt (const struct vm_space *space, busword_t virt, const void *orig, busword_t size)
{
  busword_t offset = virt &  PAGE_MASK; 
  busword_t page   = virt & ~PAGE_MASK;
  busword_t phys, len;
  busword_t size_copy = size;
  
  /* TODO: implement this faster */

  while (size)
  {
    if (!(phys = virt2phys (space, page)))
    {
      error ("untranslatable address %p\n", page);
      break;
    }
    
    if ((len = PAGE_SIZE - offset) > size)
      len = size;

    memcpy ((void *) phys + offset, orig, len);

    __vm_flush_pages (page, 1);

    orig += len;
    page += PAGE_SIZE;
    size -= len;
    
    offset = 0;
  }

  return size_copy - size;
}

int
copy2phys (const struct vm_space *space, void *dest, busword_t virt, busword_t size)
{
  busword_t offset = virt &  PAGE_MASK; 
  busword_t page   = virt & ~PAGE_MASK;
  busword_t phys, len;
  busword_t size_copy = size;
  
  /* TODO: implement this faster */

  while (size)
  {
    if (!(phys = virt2phys (space, page)))
    {
      error ("untranslatable address %p\n", page);
      break;
    }
    
    if ((len = PAGE_SIZE - offset) > size)
      len = size;

    memcpy (dest, (const void *) phys + offset, len);

    dest += len;
    page += PAGE_SIZE;
    size -= len;
    
    offset = 0;
  }

  return size_copy - size;
}

struct vm_space_region
{
  struct vm_space *space;
  struct vm_region *region;
};

static int
__map_pages (radixkey_t virt, void **phys, radixtag_t *perms, void *data)
{
  struct vm_space_region *spaceregion = data;
  radixtag_t actual_perms = (spaceregion->region->vr_access & ~VM_PAGE_PRESENT) | (*perms & VM_PAGE_PRESENT);

  /* TODO: check whether it's present or not. If it is, map it with all perms. If it is not, call __map_pages with a copy of actual perms with no write permission */
  
  return __vm_map_to (spaceregion->space->vs_pagetable, (busword_t) virt + spaceregion->region->vr_virt_start, (busword_t) *phys, 1, actual_perms);
}

int
vm_update_region (struct vm_space *space, struct vm_region *region)
{
  struct vm_space_region sr = {space, region};
  
  if (space->vs_pagetable == NULL)
    space->vs_pagetable = __vm_alloc_page_table ();

  if (region->vr_type == VREGION_TYPE_RANGEMAP)
    return __vm_map_to (space->vs_pagetable, region->vr_virt_start, region->vr_phys_start, __UNITS (region->vr_virt_end - region->vr_virt_start + 1, PAGE_SIZE), region->vr_access);
  else
    return vm_page_set_walk (REFCAST (struct vm_page_set, region->vr_page_set), __map_pages, &sr);

  return 0;
}

/* Transforms segment information into hardware translation data (i.e.
   update page tables, etc) */
int
vm_update_tables (struct vm_space *space)
{
  struct rbtree_node *this;
  struct vm_region *region;
  int ret;
  
  this = rbtree_get_first (space->vs_region_tree);
  
  while (this)
  {
    region = (struct vm_region *) rbtree_node_data (this);
    
    if ((ret = vm_update_region (space, region)) != KERNEL_SUCCESS_VALUE)
      return ret;
    
    this = rbtree_node_next (this);
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
    if (PTR_UNLIKELY_TO_FAIL (new_region = vm_region_physmap ((busword_t) this->mr_start, __UNITS ((busword_t) this->mr_end - (busword_t) this->mr_start + 1, PAGE_SIZE), VREGION_ACCESS_READ | VREGION_ACCESS_WRITE)))
    {
      error ("Failed to register VM kernel space\n");
      
      vm_space_destroy (new_space);
      return KERNEL_INVALID_POINTER;
    }
                                
    if (UNLIKELY_TO_FAIL (vm_space_overlap_region (new_space, new_region)))
    {
      error ("Failed to add region to VM kernel space\n");
      vm_region_destroy (new_region, NULL);
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

  if (UNLIKELY_TO_FAIL (vm_kernel_space_init_vremap (new_space)))
  {
    error ("Cannot map system vremap!\n");

    vm_space_destroy (new_space);

    return KERNEL_INVALID_POINTER;
  }
  
  return new_space;
}

struct vm_space *
vm_bare_sysproc_space (void)
{
  extern struct mm_region *mm_regions;
  struct vm_space  *new_space;
  
  PTR_RETURN_ON_PTR_FAILURE (new_space = vm_space_new ());

  /* Map kernel space */
  
  if (UNLIKELY_TO_FAIL (vm_kernel_space_map_image (new_space)))
  {
    error ("couldn't map kernel image\n");
    
    vm_space_destroy (new_space);
    
    return KERNEL_INVALID_POINTER;
  }

  if (UNLIKELY_TO_FAIL (vm_kernel_space_init_vremap (new_space)))
  {
    error ("Cannot map system vremap!\n");

    vm_space_destroy (new_space);

    return KERNEL_INVALID_POINTER;
  }

  return new_space;
}

static int
__load_segment_cb (struct vm_space *space, int type, int flags, busword_t virt, busword_t size, const void *data, busword_t datasize, void *opaque)
{
  struct vm_region *region;
  busword_t start_page;
  busword_t actual_size;
  busword_t actual_page_count;
  busword_t copied;

  start_page = PAGE_START (virt);
  actual_size = size + (virt - start_page);
  actual_page_count = (actual_size >> __PAGE_BITS) + !!(actual_size & PAGE_MASK);

  if (datasize > size)
  {
    error ("Segment data size overflows segment size\n");
    return -1;
  }

  /* Pages mapped by an executable must be userland pages */
  if ((region = vm_region_anonmap (start_page, actual_page_count, flags | VREGION_ACCESS_USER)) == KERNEL_INVALID_POINTER)
    return -1;

  if (vm_space_add_region (space, region) != KERNEL_SUCCESS_VALUE)
  {
    error ("Segment collision (cannot load %p-%p)\n", start_page, start_page + (actual_page_count << __PAGE_BITS) - 1);

    vm_region_destroy (region, NULL);
    
    return -1;
  }

  region->vr_role = type;
  
  if ((copied = copy2virt (space, virt, data, datasize)) != datasize)
    FAIL ("unexpected segment overrun when copying data to user (total: %d/%d)\n", copied, datasize);

  return 0;
}

int
vm_space_load_abi_vdso (struct vm_space *target, const char *abi, busword_t *abi_entry)
{
  loader_handle *handle;

  const void *abi_vdso_start;
  uint32_t    abi_vdso_size;

  if (get_abi_vdso (abi, &abi_vdso_start, &abi_vdso_size) == -1)
  {
    warning ("Cannot load abi `%s'\n", abi);

    return -1;
  }
  
  if ((handle = loader_open_exec (target, abi_vdso_start, abi_vdso_size)) == KERNEL_INVALID_POINTER)
  {
    error ("Cannot load ABI `%s': invalid ABI in initrd\n", abi);

    return -1;
  }

  if (loader_rebase (handle, USER_ABI_VDSO) == -1)
  {
    error ("Cannot rebase ABI `%s'\n", abi);

    loader_close_exec (handle);
    
    return -1;
  }

  if (loader_walk_exec (handle, __load_segment_cb, NULL) == KERNEL_ERROR_VALUE)
  {
    error ("Cannot load ABI segments\n");

    loader_close_exec (handle);
    
    return -1;
  }

  if (loader_relocate (handle) == -1)
  {
    loader_close_exec (handle);
    
    return -1;
  }

  if (abi_entry != NULL)
    *abi_entry = loader_get_exec_entry (handle);
  
  loader_close_exec (handle);

  return 0;
}

struct vm_space *
vm_space_load_from_exec (const void *exec_start, busword_t exec_size, busword_t *entry, busword_t *abi_entry)
{
  struct vm_space  *space;
  loader_handle    *handle;
  
  char abi[64];
  
  PTR_RETURN_ON_PTR_FAILURE (space = vm_bare_sysproc_space ());

  if ((handle = loader_open_exec (space, exec_start, exec_size)) == KERNEL_INVALID_POINTER)
  {
    vm_space_destroy (space);
    return KERNEL_INVALID_POINTER;
  }

  loader_get_abi (handle, abi, sizeof (abi) - 1);

  if (vm_space_load_abi_vdso (space, abi, abi_entry) == -1)
  {
    vm_space_destroy (space);
    
    loader_close_exec (handle);

    return KERNEL_INVALID_POINTER;
  }
  
  if (loader_walk_exec (handle, __load_segment_cb, NULL) == KERNEL_ERROR_VALUE)
  {
    vm_space_destroy (space);

    loader_close_exec (handle);
    
    return KERNEL_INVALID_POINTER;
  }

  space->vs_image_top = loader_get_top_addr (handle);

  /* Once all segments are properly loaded, we can perform a relocation */

  if (loader_relocate (handle) == -1)
  {
    vm_space_destroy (space);

    loader_close_exec (handle);
    
    return KERNEL_INVALID_POINTER;
  }
  
  if (entry != NULL)
    *entry = loader_get_exec_entry (handle);
  
  loader_close_exec (handle);

  return space;
}

void
vm_space_debug (struct vm_space *space)
{
  struct vm_region *region;
  struct rbtree_node *this;
  
  this = rbtree_get_first (space->vs_region_tree);
  
  while (this)
  {
    region = (struct vm_region *) rbtree_node_data (this);
    
    printk ("%y-%y: %s %H [0x%x]\n",
            region->vr_virt_start,
            region->vr_virt_end,
            region->vr_ops->name,
            region->vr_virt_end - region->vr_virt_start + 1,
            region->vr_access
      );
      
    this = rbtree_node_next (this);
  }
}

int
vm_handle_page_fault (struct task *task, busword_t addr, int access)
{
  struct vm_region *region = vm_space_find_region (REFCAST (struct vm_space, task->ts_vm_space), addr);

  if (region == KERNEL_INVALID_POINTER)
  {
    /* Send a signal, or whatever */
    return -1;
  }
  
  switch (access)
  {
  case VREGION_ACCESS_READ:
    if (region->vr_ops->read_fault == NULL)
    {
      debug ("task %d: region has no read page fault handler registered!\n", task->ts_tid);
      return -1;
    }
    
    return (region->vr_ops->read_fault) (task, region, addr);

  case VREGION_ACCESS_WRITE:
    if (region->vr_ops->write_fault == NULL)
    {
      debug ("task %d: region has no write page fault handler registered!\n", task->ts_tid);
      return -1;
    }
    return (region->vr_ops->write_fault) (task, region, addr);

  case VREGION_ACCESS_EXEC:
    if (region->vr_ops->exec_fault == NULL)
    {
      debug ("task %d: region has no exec page fault handler registered!\n", task->ts_tid);
      return -1;
    }
    
    return (region->vr_ops->exec_fault) (task, region, addr);

  default:
    FAIL ("?");
  }

  return 0;
}

/* Page set functions */
struct vm_page_set *
vm_page_set_new (void)
{
  CONSTRUCTOR_BODY_OF_STRUCT (vm_page_set);
}

static void
__vm_page_set_page_free (radixkey_t key, void **slot, radixtag_t *tag)
{
  page_free (*slot, 1);
}

/* Be careful: this operations should lock!! */
int
vm_page_set_put (struct vm_page_set *set, busword_t pageno, busword_t phys)
{
  pageno &= ~PAGE_MASK;
  
  if (radix_tree_set (&set->vp_page_tree, pageno, (void *) phys) == KERNEL_ERROR_VALUE)
    return KERNEL_ERROR_VALUE;

  if (radix_tree_set_tag (set->vp_page_tree, pageno, VM_PAGE_PRESENT) == -1)
    FAIL ("Cannot set tag on existing mapped page?\n");

  ++set->vp_pages;

  /* Check whether vp_pages == vp_limit. In that case, and if vp_template != NULL, close reference */
  return KERNEL_SUCCESS_VALUE;
}

int
vm_page_set_translate (struct vm_page_set *set, busword_t pageno, busword_t *phys)
{
  void **addr;
  radixtag_t *tag;
  
  pageno &= ~PAGE_MASK;

  /* There are no mapped pages (it may happen!) */
  if (set->vp_page_tree == NULL)
    return KERNEL_ERROR_VALUE;
    
  /* Really, this sucks */
  if ((addr = radix_tree_lookup_slot (set->vp_page_tree, pageno)) == NULL)
    return KERNEL_ERROR_VALUE;

  if ((tag = radix_tree_lookup_tag (set->vp_page_tree, pageno)) == NULL)
    return KERNEL_ERROR_VALUE;

  /* This page doesn't exist */
  if (!(*tag & VM_PAGE_PRESENT))
    return KERNEL_ERROR_VALUE;
    
  *phys = (busword_t) *addr;

  return KERNEL_SUCCESS_VALUE;
}

int
vm_page_set_walk (struct vm_page_set *set, int (*callback) (radixkey_t virt, void **phys, radixtag_t *perms, void *data), void *data)
{
  /* We have two possible approaches here:

     D := Max depth
     N := Number of pages
     A := Number of allocated pages
     
     - Walk from lowest level to highest level each radix tree and remap everything.
       Best case: A
       Worst case: N * D
       
     - Visit all pages from 0 to N.
       Best case: N
       Worst case: N * (D + log (N))
  */
  
  if (set->vp_page_tree != NULL)
    return radix_tree_walk (set->vp_page_tree, callback, data);
}

int
vm_page_set_remove (struct vm_page_set *set, busword_t pageno)
{
  radixtag_t *tag;

  pageno &= ~PAGE_MASK;
  
  if ((tag = radix_tree_lookup_tag (set->vp_page_tree, pageno)) == NULL)
    return KERNEL_ERROR_VALUE;

  if (*tag & VM_PAGE_PRESENT)
  {
    *tag &= ~VM_PAGE_PRESENT;

    --set->vp_pages;
  }
  
  return 0;
}

void
vm_page_set_destroy (struct vm_page_set *set)
{
  if (set->vp_page_tree != NULL)
    radix_tree_destroy (set->vp_page_tree, __vm_page_set_page_free);

  if (set->vp_template != NULL)
    __kernel_object_ref_close (set->vp_template);
  
  sfree (set);
}

/* vm kobjmgr callbacks */
/* TODO: implement dup () */

static void
kobjmgr_vm_space_dtor (void *data)
{
  vm_space_destroy ((struct vm_space *) data);
}

static void
kobjmgr_vm_page_set_dtor (void *data)
{
  vm_page_set_destroy ((struct vm_page_set *) data);
}

class_t vm_space_class =
{
  .name = "vm-space",
  .dtor = kobjmgr_vm_space_dtor
};

class_t vm_page_set_class =
{
  .name = "vm-page-set",
  .dtor = kobjmgr_vm_page_set_dtor
};

void
vm_init (void)
{
  struct vm_space *kernel_space;
  struct task *old_task;

  kernel_class_register (&vm_space_class);
  kernel_class_register (&vm_page_set_class);
  
  MANDATORY (SUCCESS_PTR (
               kernel_space = vm_kernel_space ()
               )
    );

  MANDATORY (SUCCESS_PTR (
               current_kctx->kc_vm_space = kernel_object_create (&vm_space_class, kernel_space)
               )
    );

  MANDATORY (SUCCESS_PTR (
	       current_kctx->kc_msgq_vremap = vm_region_vremap_new (KERNEL_MSGQ_VREMAP_START, __UNITS (KERNEL_MSGQ_VREMAP_SIZE, PAGE_SIZE), 0)
	       )
    );

  MANDATORY (SUCCESS (
	       vm_space_add_region (OBJCAST (struct vm_space, current_kctx->kc_vm_space), current_kctx->kc_msgq_vremap)
	       )
    );

  hw_vm_init ();
}

int
__alloc_colored (struct mm_region *from, struct vm_region *region, busword_t start, busword_t pages, DWORD perms)
{
  unsigned int i;
  int curr_color;
  busword_t cache_size = mm_get_cache_size ();
  busword_t virt;
  busword_t page;
  void *ptr;
  
  for (i = 0; i < pages; ++i)
  {
    virt = start + (i << __PAGE_BITS);
    
    curr_color = ADDR_COLOR (cache_size, virt);

    if ((page = mm_region_alloc_colored_page (from, curr_color)) == (busword_t) KERNEL_ERROR_VALUE)
      goto fail;

    if (vm_region_map_page (region, virt, page, VM_PAGE_PRESENT | perms) == KERNEL_ERROR_VALUE)
      goto fail;
  }

  return KERNEL_SUCCESS_VALUE;
  
fail:
  for (--i; i >= 0; --i)
  {
    virt = start + (i << __PAGE_BITS);
    
    if ((page = vm_region_translate_page (region, virt, NULL)) != (busword_t) KERNEL_ERROR_VALUE)                            
      page_free ((physptr_t) virt, 1);
  }
  
  return KERNEL_ERROR_VALUE;
}

DEBUG_FUNC (vm_region_new);
DEBUG_FUNC (vm_region_destroy);
DEBUG_FUNC (vm_region_unmap_page);
DEBUG_FUNC (vm_region_map_page);
DEBUG_FUNC (vm_region_map_pages);
DEBUG_FUNC (vm_region_translate_page);
DEBUG_FUNC (vm_space_new);
DEBUG_FUNC (vm_space_destroy);
DEBUG_FUNC (vm_space_find_first_in_range);
DEBUG_FUNC (vm_test_range);
DEBUG_FUNC (__invalidate_pages);
DEBUG_FUNC (vm_region_invalidate);
DEBUG_FUNC (vm_space_add_region);
DEBUG_FUNC (vm_space_overlap_region);
DEBUG_FUNC (vm_space_find_region);
DEBUG_FUNC (virt2phys);
DEBUG_FUNC (copy2virt);
DEBUG_FUNC (__map_pages);
DEBUG_FUNC (vm_update_region);
DEBUG_FUNC (vm_update_tables);
DEBUG_FUNC (vm_kernel_space);
DEBUG_FUNC (vm_bare_sysproc_space);
DEBUG_FUNC (__load_segment_cb);
DEBUG_FUNC (vm_space_load_from_exec);
DEBUG_FUNC (vm_space_debug);
DEBUG_FUNC (vm_handle_page_fault);
DEBUG_FUNC (vm_page_set_new);
DEBUG_FUNC (__vm_page_set_page_free);
DEBUG_FUNC (vm_page_set_put);
DEBUG_FUNC (vm_page_set_translate);
DEBUG_FUNC (vm_page_set_walk);
DEBUG_FUNC (vm_page_set_remove);
DEBUG_FUNC (vm_page_set_destroy);
DEBUG_FUNC (kobjmgr_vm_space_dtor);
DEBUG_FUNC (kobjmgr_vm_page_set_dtor);
DEBUG_FUNC (vm_init);
DEBUG_FUNC (__alloc_colored);
