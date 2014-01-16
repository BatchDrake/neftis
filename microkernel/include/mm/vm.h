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
    
#ifndef _MM_VM_H
#define _MM_VM_H

#include <types.h>
#include <misc/list.h>
#include <mm/regions.h>

/* 
 * NEFTIS VIRTUAL MEMORY SUBSYSTEM - An epic and vigorous prose by BatchDrake
 * --------------------------------------------------------------------------
 *
 * Neftis should be able to manage at least the following eight region types:
 *
 * VREGION_TYPE_ANON
 *    This type of region defines a cache-friendly set of pages that are
 *    susceptible to be stolen by a swapper process. Page faults in
 *    this region are told to the above mentioned daemon in order to 
 *    get back the the absent page, and probably some more with it.
 *
 * VREGION_TYPE_ANON_NOSWAP
 *    This region, like VREGION_TYPE_ANON, defines a cache-friendly set of 
 *    pages, but in an unswappable way. In other words, these pages can't
 *    be stolen by swapper.
 *
 * VREGION_TYPE_SHARED
 *    Shared region of memory among processes. This really points to an
 *    external structure that stores information about where to locate
 *    the actual data within the cluster, and handle page faults to make
 *    access possible. Memory writes to this area outside the current
 *    machine should be serialized in some manner to avoid cache-inconsistent
 *    copies. Maybe sending broadcast signals of "page invalidate" or 
 *    something similar.
 *
 * VREGION_TYPE_COPYONWRITE
 *    Copy-on-write memory. This region references another region, and page
 *    faults will be managed as follows:
 *    
 *    Read-access page fault: this will call the read-access page fault
 *    handler of the original region, causing it to be immediatly available.
 *
 *    Write-access page fault: this will allocate a new page in a
 *    cache-friendly way and copying the original contents to it.
 *
 *    Copy-on-write regions are of the same size than the original one, and
 *    can only reference anon regions and other copy-on-write regions.
 *
 * VREGION_TYPE_IOMAP
 *    Defines a I/O dedicated region, physically mapped to an external device.
 *    Processes with this kind of regions shouldn't be able to migrate.
 *
 * VREGION_TYPE_ZERO
 *    In a similar way to Copy-on-Write regions, this defines a set of pages
 *    that point to the same page frame, filled up with zeroes. Any write
 *    against this region will allocate a new page.
 *
 * VREGION_TYPE_KERNEL
 *    Defines a 1:1 mapped area. This region is not migrated and exists just
 *    for architecture-dependant stuff related to context-switching.
 *    Despite the characteristics of this region, this makes the destination
 *    node impose a restriction: if the migrated process has no room for
 *    its own kernel regions, the process will be rejected.
 *
 * VREGION_TYPE_CUSTOM
 *    Non-allocated region with a subtype and custom page-fault handlers.
 *    Processes with this region can only migrate to nodes with the
 *    appropiate software to handle it (and only if this software allows
 *    migration).
 */
 
#define VREGION_TYPE_ANON        0 
#define VREGION_TYPE_ANON_NOSWAP 1 
#define VREGION_TYPE_SHARED      2
#define VREGION_TYPE_COPYONWRITE 3 
#define VREGION_TYPE_IOMAP       4
#define VREGION_TYPE_ZERO        5
#define VREGION_TYPE_KERNEL      6 
#define VREGION_TYPE_CUSTOM      7 

#define VREGION_ACCESS_READ      (1 << 1)
#define VREGION_ACCESS_WRITE     (1 << 2)
#define VREGION_ACCESS_EXEC      (1 << 3)

#define VM_PAGE_PRESENT          1
#define VM_PAGE_READABLE         VREGION_ACCESS_READ
#define VM_PAGE_WRITABLE         VREGION_ACCESS_WRITE
#define VM_PAGE_EXECUTABLE       VREGION_ACCESS_EXEC
#define VM_PAGE_ACCESSED         16
#define VM_PAGE_DIRTY            32
#define VM_PAGE_KERNEL           64  /* Keep always in TLB */
#define VM_PAGE_IOMAP            128 /* Don't cache this */

/* Uses cache-coloring */
struct vanon_strip
{
  SORTED_LIST;
  
  int       vs_ref_cntr;
  
  busword_t vs_pages;
  busword_t vs_virt_start;
  busword_t vs_phys_start;
  
  struct    vm_region *vs_region;
};

struct vm_region
{
  SORTED_LIST;
  
  int   vr_type;
  
  DWORD vr_access;
  
  busword_t vr_virt_start;
  busword_t vr_virt_end;
  
  union
  {
    struct vanon_strip *anon;
  }
  vr_strips;
};

struct vm_space
{
  struct vm_region *vs_regions;
  
  /* This is hardware dependant */
  void *vs_pagetable;
};

struct vm_region *vm_region_new (int);
void vm_region_destroy (struct vm_region *);
struct vm_space *vm_space_new (void);
struct vanon_strip *vm_alloc_colored (struct mm_region *, busword_t, int);
void vm_space_destroy (struct vm_space *);
void vm_region_invalidate (struct vm_region *);
struct vm_region *vm_region_shared (busword_t, busword_t, busword_t);
int vm_space_add_region (struct vm_space *, struct vm_region *);
int vm_space_overlap_region (struct vm_space *, struct vm_region *);
int vm_update_region (struct vm_space *, struct vm_region *);
int vm_update_tables (struct vm_space *);
struct vm_space *vm_kernel_space (void);
const char *vm_type_to_string (int);
void vm_space_debug (struct vm_space *);
void vm_init (void);
#endif /* _MM_VM_H */

