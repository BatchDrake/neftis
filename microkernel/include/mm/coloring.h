/*
 *    Definitions to handle cache coloring
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
    
#ifndef _MM_COLORING_H
#define _MM_COLORING_H

#include <types.h>
#include <mm/regions.h>

#define PAGE_COLOR_BITS(cache_size) ((cache_size) / PAGE_SIZE)

#define PAGE_COLOR_MASK(cache_size) \
  ((1 << PAGE_COLOR_BITS (cache_size)) - 1)

#define PAGE_AVAIL_COLORS(cache_size) (1 << PAGE_COLOR_BITS (cache_size))

#define PAGE_COLOR(cache_size, page_no) \
  ((page_no) & PAGE_COLOR_MASK (cache_size))

#define ADDR_COLOR(cache_size, addr) \
  (PAGE_NO (addr) & PAGE_COLOR_MASK (cache_size))

void mm_set_cache_size (int);
int mm_get_cache_size (void);
int mm_available_colors (void);
busword_t mm_get_colored_page (struct mm_region *, int);

#endif /* _MM_COLORING_H */

