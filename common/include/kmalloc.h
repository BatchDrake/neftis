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
    
#ifndef _MM_KMALLOC_H
#define _MM_KMALLOC_H

#include <util.h>

#define KMALLOC_HEADER_MAGIC 0xda7ab10c
#define KMALLOC_CANARY       0xdead7007

#define KMALLOC_FREE_CHUNK 0
#define KMALLOC_BUSY_CHUNK 1

#define MEMORY_BLOCK_SIZE  16

struct kas_hdr
{
  DWORD             kh_magic;
  void             *kh_start;
  void             *kh_end;
    
  struct kas_chunk *kh_first;
  struct kas_chunk *kh_last;
} ALIGNED (MEMORY_BLOCK_SIZE);

struct kas_chunk
{
  DWORD                kc_canary;
  int                  kc_state;
  size_t               kc_size;
  
  struct kas_chunk    *kc_next;
  struct kas_chunk    *kc_prev;

} ALIGNED (MEMORY_BLOCK_SIZE);

void* kmalloc (size_t size, struct kas_hdr *hdr);
void* kmalloc_fast (size_t size, struct kas_hdr *hdr);
void  kmfree (void *ptr, struct kas_hdr *hdr);

#endif /* _MM_KMALLOC_H */

