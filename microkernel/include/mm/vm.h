/*
 *    Atomik's virtual memory subsystem.
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
    
#ifndef _MM_VM_H
#define _MM_VM_H

#include <types.h>
#include <misc/list.h>
#include <misc/object.h>
#include <misc/radix_tree.h>
#include <mm/regions.h>

#define VREGION_TYPE_RANGEMAP    0
#define VREGION_TYPE_PAGEMAP     1

#define VREGION_ROLE_USERMAP     0
#define VREGION_ROLE_STACK       1
#define VREGION_ROLE_KERNEL      2

#define VREGION_ACCESS_READ      (1 << 1)
#define VREGION_ACCESS_WRITE     (1 << 2)
#define VREGION_ACCESS_EXEC      (1 << 3)

#define VREGION_ACCESS_USER      (1 << 8)

#define VM_PAGE_PRESENT          1
#define VM_PAGE_READABLE         VREGION_ACCESS_READ
#define VM_PAGE_WRITABLE         VREGION_ACCESS_WRITE
#define VM_PAGE_EXECUTABLE       VREGION_ACCESS_EXEC
#define VM_PAGE_ACCESSED         16
#define VM_PAGE_DIRTY            32
#define VM_PAGE_KERNEL           64  /* Keep always in TLB */
#define VM_PAGE_IOMAP            128 /* Don't cache this */
#define VM_PAGE_USER             VREGION_ACCESS_USER /* Usermode page */
#define VM_PAGE_ALLFLAGS         513

/* Please NOTE: this new approach for managing virtual memory
   regions knows NOTHING about the structures used to store
   page information. API will have functions like "map_range",
   but that's all. */
/*
 * This is how we deal with shared memory ranges:
 * - shared->init allocates a new range, and sets the reference count to 1
 * - When shared->dup is called upon this range, we copy the pointers from
 *   the original range and increment the reference count. We also use the
 *   tree from the original range (just copy the pointer). We also add a
 *   list of clients of this region.
 * - Resize operation goes through every client of this region, and determines
 *   the highest (and lowest) virtual address. If we leave some pages totally
 *   inaccessible, we can free them.
 * - Destroying it decrements reference count
 * - If reference count goes to zero, we free everything.
 */

struct task;
struct vm_region;

struct vm_region_ops
{
  char *name;

  /* Initialize this region */
  int (*init)        (struct task *, struct vm_region *, void *);

  /* Destroy this region */
  int (*destroy)     (struct task *, struct vm_region *);

  /* Resize region (grow / shrink) */
  int (*resize)      (struct task *, struct vm_region *, busword_t, busword_t);

  /* Duplicate region */
  int (*dup)         (struct task *, struct vm_region *, struct vm_region *);

  /* Page fault handlers */
  int (*read_fault)  (struct task *, struct vm_region *, busword_t);
  int (*write_fault) (struct task *, struct vm_region *, busword_t);
  int (*exec_fault)  (struct task *, struct vm_region *, busword_t);
};

/* This structure holds a radix tree of pages. The key is the offset from
   the segment they should be mapped against.

   VMO's are just regular vm_region whose vm_page_set can be modified
   by a process.

   NOTE: all page sets *MUST* be accessed from kobjmgr as we need
   to keep track of all tasks with references to this page set
   in order to update their pagetables upon page set update */

struct vm_page_set
{
  KERNEL_OBJECT;

  busword_t vp_pages; /* Count of allocated pages */
  busword_t vp_limit; /* Page limit */
  
  struct radix_tree_node *vp_page_tree; /* Radix tree of pages */

  objref_t *vp_template; /* Template (for copy-on-write) */
};

/* I don't care too much about this. I don't expect a process to
   have thousands of regions, but if it ends up becoming like that,
   vm_region will be inside a tree in future versions */
struct vm_region
{
  SORTED_LIST;

  KERNEL_OBJECT;
  
  int                   vr_type;
  int                   vr_role;
  int                   vr_unlinked_remap; /* Underlying physical pages are not mapped to kernel space */
  
  struct vm_region_ops *vr_ops;
  void                 *vr_ops_data; /* Opaque data */
  
  DWORD                 vr_access;
  
  busword_t vr_virt_start;
  busword_t vr_virt_end;

  union
  {
    /* This is just an optimization: some maps are just remaps
       of existing physical regions to virtual regions */
    busword_t  vr_phys_start;

    /* This is the set of pages that are mapped in RAM */
    objref_t  *vr_page_set;
  };
};

struct vm_space
{
  struct vm_region *vs_regions;
  /* This is hardware dependant */
  void *vs_pagetable;
};

/* Region operations */
struct vm_region *vm_region_new (busword_t, busword_t, struct vm_region_ops *, void *);
int vm_region_map_page (struct vm_region *, busword_t, busword_t, DWORD);
int vm_region_unmap_page (struct vm_region *, busword_t);
int vm_region_map_pages (struct vm_region *, busword_t, busword_t, DWORD, busword_t);
busword_t vm_region_translate_page (struct vm_region *, busword_t, DWORD *);

void vm_region_invalidate (struct vm_region *);
void vm_region_destroy (struct vm_region *, struct task *);

/* Space operations */
struct vm_space *vm_space_new (void);
struct vm_region *vm_space_find_first_in_range (struct vm_space *, busword_t, busword_t);
int vm_space_add_region (struct vm_space *, struct vm_region *);
int vm_space_overlap_region (struct vm_space *, struct vm_region *);
int vm_update_region (struct vm_space *, struct vm_region *);
int vm_update_tables (struct vm_space *);
void vm_space_destroy (struct vm_space *);

struct vm_space *vm_kernel_space (void);
struct vm_space *vm_bare_sysproc_space (void);
struct vm_space *vm_space_load_from_exec (const void *, busword_t, busword_t *);

void vm_space_debug (struct vm_space *);

/* Page set operations */
struct vm_page_set *vm_page_set_new (void);
int  vm_page_set_put (struct vm_page_set *set, busword_t pageno, busword_t phys);
int  vm_page_set_remove (struct vm_page_set *set, busword_t pageno);
int  vm_page_set_translate (struct vm_page_set *set, busword_t pageno, busword_t *phys);
void vm_page_set_destroy (struct vm_page_set *set);

/* Misc operations */
busword_t virt2phys (const struct vm_space *space, busword_t virt);
int copy2virt (const struct vm_space *, busword_t, const void *, busword_t);
int copy2phys (const struct vm_space *, void *, busword_t, busword_t);
int vm_handle_page_fault (struct task *, busword_t, int);
void vm_init (void);

int __alloc_colored (struct mm_region *, struct vm_region *, busword_t, busword_t, DWORD);

#endif /* _MM_VM_H */

