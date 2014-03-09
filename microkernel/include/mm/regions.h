/*
 *    Structures to handle physical memory regions
 *    Copyright (C) 2010 Gonzalo J. Carracedo <BatchDrake@gmail.com>
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
    
#ifndef _MM_REGIONS_H
#define _MM_REGIONS_H

#include <types.h>
#include <lock/lock.h>

#define MM_REGION_BUCKETS (BUSWORD_BITS - __PAGE_BITS)

struct page
{
  struct page  *free_prev;
  struct page  *free_next;
  unsigned long index;
  
  int refcount;
};

struct free_bucket
{
  struct page *first;
  char *map;
};

struct mm_region
{
  struct mm_region  *mr_next;
  spin_t             mr_lock;
  
  physptr_t          mr_start;
  struct page       *mr_page_list;
  
  physptr_t          mr_end;
  
  memsize_t          mr_pages;

  struct free_bucket mr_buckets[MM_REGION_BUCKETS];
};

#define MMR_PAGE_TO_ADDR(mr, relpg)                      \
  (PAGE_START (mr->mr_start) + PAGE_ADDR (relpg))

#define MMR_ADDR_TO_PAGE(mr, addr)                       \
  PAGE_NO ((busword_t) (addr) - (busword_t) mr->mr_start)
  
#define MMR_OWNS_ADDR(mr, addr)                          \
  (((busword_t) addr >= (busword_t) mr->mr_start) &&     \
  ((busword_t) addr <= (busword_t) mr->mr_end))



void mm_register_region (physptr_t, physptr_t);
physptr_t page_alloc (memsize_t);
int page_free (void *, memsize_t);
busword_t mm_region_alloc_colored_page (struct mm_region *, int);
unsigned long mm_region_debug (struct mm_region *);

#endif /* _MM_REGIONS_H */

