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
    
#ifndef _MM_SPALLOC_H
#define _MM_SPALLOC_H

#include <types.h>

#define SPALLOC_BITMAP_ATOM DWORD
#define SPALLOC_BLOCK_SIZE  sizeof (SPALLOC_BITMAP_ATOM)
#define SPALLOC_BLOCK_BITS  (SPALLOC_BLOCK_SIZE * 8)

#define SPALLOC_CANARY      0xcabe2ac0


struct spalloc_page
{
  WORD                  sp_pages;
  WORD                  sp_unused;
  DWORD                 sp_biggest;
  DWORD                 sp_bigptr;
  struct spalloc_page  *sp_next;
  struct spalloc_page  *sp_prev;
} PACKED_ALIGNED (32);

struct spalloc_prefix
{
  DWORD                 sp_canary;
  DWORD                 sp_size;
  struct spalloc_page  *sp_owner;
} PACKED;

#define SPALLOC_BLOCKS_PER_PAGE (PAGE_SIZE / SPALLOC_BLOCK_SIZE)

#define SPALLOC_HEADER_BLOCKS                                   \
  __UNITS (sizeof (struct spalloc_page), SPALLOC_BLOCK_SIZE)

#define SPALLOC_PREFIX_BLOCKS                                   \
  __UNITS (sizeof (struct spalloc_prefix), SPALLOC_BLOCK_SIZE)

#define SPALLOC_BITMAP_SIZE(hdr)                                \
  __UNITS (SPALLOC_BLOCKS_PER_PAGE * (hdr)->sp_pages, SPALLOC_BLOCK_BITS)

#define SPALLOC_BITMAP_START(hdr)                               \
  ((SPALLOC_BITMAP_ATOM *) ((hdr) + 1))

#define SPALLOC_MARK_BLOCK(hdr, block)                          \
  SPALLOC_BITMAP_START(hdr)[(block) / SPALLOC_BLOCK_BITS] |=    \
  (1 << ((block) % SPALLOC_BLOCK_BITS))

#define SPALLOC_UNMARK_BLOCK(hdr, block)                        \
  SPALLOC_BITMAP_START(hdr)[(block) / SPALLOC_BLOCK_BITS] &=    \
  ~(1 << ((block) % SPALLOC_BLOCK_BITS))

#define SPALLOC_BLOCK_STATE(hdr, block)                         \
  (!!(SPALLOC_BITMAP_START(hdr)[(block) / SPALLOC_BLOCK_BITS] & \
    (1 << ((block) % SPALLOC_BLOCK_BITS))))

void *spalloc (size_t);
void spfree (void *ptr);

#endif /* _MM_SPALLOC_H */

