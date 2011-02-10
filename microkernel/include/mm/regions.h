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

struct mm_region
{
  struct mm_region *mr_next;
  physptr_t         mr_start;
  physptr_t         mr_end;
  
  memsize_t         mr_size;
  memsize_t         mr_pages;
  
  memsize_t         mr_last_freed;
  
  /* !!!TODO: incluir un semáforo aquí. */
  /* !!!TODO: si hay varias memorias, hacer una especie de
     sem_trylock y comprobar otras áreas que estén libres */
  
  spin_t            mr_lock;     
  uint8_t          *mr_bitmap;
};

#define MMR_BITMAP_SIZE(mr)  __UNITS(mr->mr_pages, 8)
#define MMR_BITMAP_PAGES(mr) __UNITS(MMR_BITMAP_SIZE  (mr), PAGE_SIZE)

#define MMR_PAGE_TO_ADDR(mr, relpg)                      \
  (PAGE_START (mr->mr_start) + PAGE_ADDR (relpg))

#define MMR_ADDR_TO_PAGE(mr, addr)                       \
  PAGE_NO ((busword_t) (addr) - (busword_t) mr->mr_start)
  
#define MMR_OWNS_ADDR(mr, addr)                          \
  (((busword_t) addr >= (busword_t) mr->mr_start) &&     \
  ((busword_t) addr <= (busword_t) mr->mr_end))

#define MMR_PAGE_STATE(mr, relpg)                        \
  (!!(mr->mr_bitmap[relpg >> 3] & (1 << (relpg & 7))))
#define MMR_MARK_PAGE(mr, relpg)                         \
  mr->mr_bitmap[relpg >> 3] |= (1 << (relpg & 7))
#define MMR_UNMARK_PAGE(mr, relpg)                       \
  mr->mr_bitmap[relpg >> 3] &= ~(1 << (relpg & 7))

void mm_register_region (physptr_t, physptr_t);
physptr_t page_alloc (memsize_t pages);

#endif /* _MM_REGIONS_H */

