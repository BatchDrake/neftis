/*
 *    Virtual remap.
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
 
#ifndef _MM_VREMAP_H
#define _MM_VREMAP_H

#include <asm/layout.h>

/* Sorted in size */
struct vremap_free_chunk
{
  SORTED_LIST;

  busword_t address;
  busword_t pages;
};

struct vremap_data
{
  struct vremap_free_chunk *chunk_list_head;
  
  
  busword_t address;
  busword_t pages_total;
  busword_t pages_busy;
};

void vremap_init (void);

struct vm_region *vm_region_vremap_new (busword_t, busword_t, DWORD);
busword_t vm_region_vremap_ensure (struct vm_region *, busword_t);
int vm_region_vremap_release (struct vm_region *, busword_t, busword_t);
busword_t kernel_vremap_ensure (busword_t);
int kernel_vremap_release (busword_t, busword_t);
void vremap_data_debug (struct vremap_data *);
int vm_kernel_space_init_vremap (struct vm_space *);
int kernel_vremap_map_pages (busword_t, busword_t, busword_t, DWORD);
void kernel_vremap_update_kernel (void);
int vm_region_is_vremap (struct vm_region *);

#endif /* _MM_VREMAP_H */
