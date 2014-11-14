/*
 *    Virtual memory anonymous mapping.
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

#include <mm/regions.h>
#include <mm/coloring.h>
#include <mm/vm.h>
#include <mm/anon.h>
#include <mm/salloc.h>

#include <misc/list.h>
#include <task/loader.h>

#include <arch.h>
#include <kctx.h>

static int
anonmap_pagefault (struct task *task, struct vm_region *region, busword_t failed_addr)
{
  if (task == NULL)
    error ("anonmap pagefault while in kernel mode: %p\n", failed_addr);
  else
    error ("segmentation fault: %p\n", failed_addr);

  return -1;
}

static int
kmap_pagefault (struct task *task, struct vm_region *region, busword_t failed_addr)
{
  error ("Kernel region fault: %p\n", failed_addr);
 
  return -1;
}

struct vm_region_ops anonmap_region_ops =
{
  .name        = "anon",
  .read_fault  = anonmap_pagefault,
  .write_fault = anonmap_pagefault,
  .exec_fault  = anonmap_pagefault
};

struct vm_region_ops kmap_region_ops =
{
  .name        = "kernel",
  .read_fault  = kmap_pagefault,
  .write_fault = kmap_pagefault,
  .exec_fault  = kmap_pagefault
};

class_t vm_page_set_class;

struct vm_region *
vm_region_anonmap (busword_t virt, busword_t pages, DWORD perms)
{
  struct vm_region *new;
  struct mm_region *region;
  struct vm_page_set *pageset;

  extern struct mm_region *mm_regions;

  DECLARE_CRITICAL_SECTION (alloc_anon);
  
  /* TODO: do this NUMA-friendly */

  PTR_RETURN_ON_PTR_FAILURE (pageset = vm_page_set_new ());
  
  if ((new = vm_region_new (virt, virt + (pages << __PAGE_BITS) - 1, &anonmap_region_ops, NULL)) == KERNEL_INVALID_POINTER)
    return KERNEL_INVALID_POINTER;

  new->vr_access = perms;
  new->vr_type   = VREGION_TYPE_PAGEMAP;

  /* Non-shared page sets are owned by kernel */
  if ((new->vr_page_set = kernel_object_instance_task (&vm_page_set_class, pageset, NULL)) == NULL)
  {
    vm_page_set_destroy (pageset);
    vm_region_destroy (new, NULL);

    return KERNEL_INVALID_POINTER;
  }
  
  /* TODO: use mutexes, this can wait */
  CRITICAL_ENTER (alloc_anon);
  
  region = mm_regions;

  while (region != NULL)
  {
    if (__alloc_colored (region, new, virt, pages, perms) == KERNEL_SUCCESS_VALUE)
      break;
      
    region = region->mr_next;
  }

  CRITICAL_LEAVE (alloc_anon);
  
  if (PTR_UNLIKELY_TO_FAIL (region))
  { 
    vm_region_destroy (new, NULL);
    return KERNEL_INVALID_POINTER;
  }

  return new;
}

struct vm_region *
vm_region_remap (busword_t virt, busword_t phys, busword_t pages, DWORD perms)
{
  struct vm_region *new;
  busword_t vaddr, paddr;
  
  int i;

  PTR_RETURN_ON_PTR_FAILURE (new = vm_region_new (virt, virt + (pages << __PAGE_BITS) - 1, &kmap_region_ops, NULL));

  new->vr_type   = VREGION_TYPE_RANGEMAP;
  new->vr_access = perms | VM_PAGE_PRESENT;
  new->vr_phys_start = phys;
  
  return new;
  
fail:
  vm_region_destroy (new, NULL);
  
  return KERNEL_INVALID_POINTER;
}

struct vm_region *
vm_region_physmap (busword_t phys, busword_t pages, DWORD perms)
{
  return vm_region_remap (phys, phys, pages, perms);
}

/* User for allocating stack regions. This is basically an anonmap
   that starts at stack_bottom - (pages << __PAGE_BITS) and ends
   at stack_bottom - 1. This means that if we want a stack bottom at
   0xcfffffff (Linux-x86 style), we need to call:

   vm_region_stack (0xd0000000, 16)

   Which, under x86, allocates 64 KiB of stack right below of the
   kernel space
  */

struct vm_region *
vm_region_stack (busword_t stack_bottom, busword_t pages)
{
  busword_t top_page = stack_bottom - (pages << __PAGE_BITS);
  struct vm_region *new;

  PTR_RETURN_ON_PTR_FAILURE (new = vm_region_anonmap (top_page, pages, VREGION_ACCESS_READ | VREGION_ACCESS_WRITE | VREGION_ACCESS_USER));

  new->vr_role   = VREGION_ROLE_STACK;

  return new;
}


/* TODO: design macros to generalize dynamic allocation */
struct vm_region *
vm_region_iomap (busword_t virt, busword_t phys, busword_t pages)
{
  return vm_region_remap (virt, phys, pages, VREGION_ACCESS_READ | VREGION_ACCESS_WRITE | VM_PAGE_KERNEL);
}

DEBUG_FUNC (anonmap_pagefault);
DEBUG_FUNC (kmap_pagefault);
DEBUG_FUNC (vm_region_anonmap);
DEBUG_FUNC (vm_region_remap);
DEBUG_FUNC (vm_region_physmap);
DEBUG_FUNC (vm_region_stack);
DEBUG_FUNC (vm_region_iomap);
