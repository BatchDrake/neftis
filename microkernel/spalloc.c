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

#include <types.h>
#include <string.h>
#include <util.h>

#include <mm/regions.h>
#include <mm/spalloc.h>

/* TODO: Sparse allocation can be significantly accelerated if we use
   trylocks instead of regular locks. The selected page can be busy, but
   that shouldn't prevent us of searching chunks in other pages. */
   
struct spalloc_page *spalloc_first = NULL;
spin_t spalloc_lock = SPINLOCK_UNLOCKED;

INLINE size_t
spalloc_bytes_to_blocks (size_t bytes)
{
  return __UNITS (bytes, SPALLOC_BLOCK_SIZE);
}

struct spalloc_page *
spalloc_find_suitable_page (size_t blocks)
{
  struct spalloc_page *this;

  this = spalloc_first;

  while (this)
  {
    if (this->sp_biggest >= blocks)
      return this;

    this = this->sp_next;
  }

  return NULL;
}

void
spalloc_upgrade_biggest (struct spalloc_page *page)
{
  DWORD i, meta_data_blocks, biggest;
  
  meta_data_blocks = SPALLOC_HEADER_BLOCKS + SPALLOC_BITMAP_SIZE (page);
  
  biggest = 0;
  
  page->sp_biggest = 0;
  page->sp_bigptr  = (DWORD) -1;
  
  for (i = meta_data_blocks; i < page->sp_pages * SPALLOC_BLOCKS_PER_PAGE; i++)
  {
    if (SPALLOC_BLOCK_STATE (page, i))
    {
      if (biggest > page->sp_biggest)
      {
        page->sp_biggest = biggest;
        page->sp_bigptr  = i - biggest;
      }
      
      biggest = 0;
    }
    else
      biggest++;
  }
  
  if (biggest > page->sp_biggest)
  {
    page->sp_biggest = biggest;
    page->sp_bigptr  = i - biggest;
  }
}

/*
      How do we obtain how many blocks inside a page we require in order
      to handle a given amount X of usable data blocks.

      Let T the total amount required including metadata,
          K the size of header (in blocks),
          B the size of the bitmap (in blocks) and
          X the amount of usable blocks we need. So:

      
      T = K + B + X

      Let S: SPALLOC_BLOCK_BITS (bits per block). What we need is:
 
      B = __ALIGN (T, S);
      
      Or the same thing:

      B = (T + S - 1) \ S; ("\" is really integer div)

      At this point, we can be sure of this:
      
      B * S <= T + S - 1 < (B + 1) * S
      
      Or, in two prepositions:
      
      B * S <= T + S - 1
      (B + 1) * S > T + S - 1
      
      B * S <= T + S - 1
      B * S + S> T + S - 1
      
      B * S <= T + S - 1
      B * S > T + S - 1 - S
      
      B * S <= T + S - 1
      B * S > T - 1
      
      This means:
      
      B * S <= T + S - 1
      B * S >= T
      
      If we replace T with the expression above:
      
      B * S <= K + B + X + S - 1
      B * S >= K + B + X
      
      With this, we have the maximum and minimum values for B * S. The
      relevant expression is the one who sets the upper bound for B * S,
      because is the one we will use to estimate how many blocks
      (including metadata blocks) we need for a given amount X of 
      usable blocks:
      
      B * S <= K + B + X + S - 1
     
      We're going to do some calculus here:
      
      B * (S - 1) <= K + X + S - 1
      
      B <= (K + X + S - 1) / (S - 1)
      
      B <= (K + X) / (S - 1) + 1
      
      So, we assume an upper bound of (K + X) / (S - 1) + 1. By replacing
      in the expression of T, we get the following:
      
      T = K + (K + X) / (S - 1) + X + 1
      Is this approximation good? Let's try:
      
      Page size is 4096, block size 32 bits -> 4 bytes. So we have
      1024 blocks in each page. A page will have 8 blocks for the header,
      and 1024 / 32 = 32 blocks for the bitmap. This leaves 984 usable blocks.
      
      So, if we request 984 blocks, we will have this:
      
      T = 8 + (8 + 984) / 31 + 984 + 1
      T = 8 + (992) / 31 + 984 + 1 = 1025 (exactly)
      
      But this is too much waste! If we substract 1 to that expression,
      would it work too? Let's check:
      
      Assuming B = (K + X) / (S - 1)
      
      (K + X) / (S - 1) * S <= K + (K + X) / (S - 1) + X + S - 1
      (K + X) / (S - 1) * S >= K + (K + X) / (S - 1) + X
      
      (K + X) / (S - 1) * (S - 1) <= K + X + S - 1
      (K + X) / (S - 1) * (S - 1) >= K + X
      
      Or the same thing:
      
      (K + X) / (S - 1) <= (K + X) / (S - 1) + 1
      
      Which is clearly a tautology. Even if we assume integer divisions,
      we can deduce the same thing. So, let's try with (K + X) / (S - 1):
      
      T = 8 + (8 + 984) / 31 + 984
      T = 8 + (992) / 31 + 984 = 1024 (exactly)
      
      And, what happens if we request 985?
      
      T = 8 + (8 + 985) / 31 + 985
      T = 8 + (993) / 31 + 985 = 1025,032... 
      
      Which drops us automatically to a second page.
      
      We don't want this to be exact. The only points of interest are in
      those precise values that mark the difference between
      one page more or not.
      
      In fact, this new expression (if we didn't align it) would fail
      miserably in every value before the (31 - 8) blocks:
      
      B = (8 + (31 - 8)) / 31 = 0
      
      0 bits for bitmap! 
      */
      
struct spalloc_page *
spalloc_new_page (size_t blocks)
{
  size_t meta_data_blocks;
  size_t total_blocks;
  size_t pages;
  int i;
  
  struct spalloc_page *result;
     
  meta_data_blocks = SPALLOC_HEADER_BLOCKS +
    (SPALLOC_HEADER_BLOCKS + blocks) / (SPALLOC_BLOCK_BITS - 1);
  
  /* First approximation */
  
  total_blocks     = __ALIGN (meta_data_blocks + blocks,
                              SPALLOC_BLOCKS_PER_PAGE);

  
  /* The actual value with the aligned total amount of blocks */
  meta_data_blocks = SPALLOC_HEADER_BLOCKS +
                     __UNITS (total_blocks, SPALLOC_BLOCK_BITS);
  
  pages = total_blocks / SPALLOC_BLOCKS_PER_PAGE;
  
  if (pages >= (1 << sizeof (WORD)))
  {
    /* Bug! Too much. */

    return NULL;
  }

  if ((result = page_alloc (pages)) == NULL)
    return NULL;
  
  result->sp_pages   = (WORD)  pages;
  result->sp_biggest = (DWORD) (total_blocks - meta_data_blocks);
  result->sp_bigptr  = (DWORD) (meta_data_blocks);

  for (i = 0; i < meta_data_blocks; i++)
    SPALLOC_MARK_BLOCK (result, i);

  for (i = meta_data_blocks; i < total_blocks; i++)
    SPALLOC_UNMARK_BLOCK (result, i);

  return result;
}

void
spalloc_page_register (struct spalloc_page *page)
{
  if (spalloc_first)
    spalloc_first->sp_prev = page;
  
  page->sp_next = spalloc_first;
  page->sp_prev = NULL;
  
  spalloc_first = page;
}

void *
spalloc (size_t size)
{
  size_t blocks;
  struct spalloc_page *page;
  struct spalloc_prefix *prefix;
  
  SPALLOC_BITMAP_ATOM *atoms;
  DWORD i, bigptr;
  
  blocks = SPALLOC_PREFIX_BLOCKS + spalloc_bytes_to_blocks (size);
  
  spin_lock (&spalloc_lock);

  if ((page = spalloc_find_suitable_page (blocks)) == NULL)
  {
    if ((page = spalloc_new_page (blocks)) == NULL)
    {
      spin_unlock (&spalloc_lock);
      return NULL;
    }
    
    spalloc_page_register (page);
  }
  
  bigptr = page->sp_bigptr;
  atoms  = (SPALLOC_BITMAP_ATOM *) page;
  
  for (i = 0; i < blocks; i++)
    SPALLOC_MARK_BLOCK (page, i + bigptr);
  
  prefix = (struct spalloc_prefix *) &atoms[bigptr];
  
  prefix->sp_canary = SPALLOC_CANARY;
  prefix->sp_size   = blocks;
  prefix->sp_owner  = page;
  
  spalloc_upgrade_biggest (page);
  
  spin_unlock (&spalloc_lock);
  
  return (void *) &atoms[SPALLOC_PREFIX_BLOCKS + bigptr];
}

void
spfree (void *ptr)
{
  SPALLOC_BITMAP_ATOM *atoms;
  struct spalloc_prefix *prefix;
  struct spalloc_page *page;
  DWORD block, i;
  busword_t diff;
  
  spin_lock (&spalloc_lock);
  
  atoms = (SPALLOC_BITMAP_ATOM *) ptr;
  
  prefix = (struct spalloc_prefix *) &atoms[-SPALLOC_PREFIX_BLOCKS];
  
  if (prefix->sp_canary != SPALLOC_CANARY)
    FAIL ("spfree found dead canary! wtf!\n");
  
  page = prefix->sp_owner;
  
  diff = (busword_t) prefix - (busword_t) page;
  
  block = diff / sizeof (SPALLOC_BLOCK_SIZE);
  
  for (i = 0; i < prefix->sp_size; i++)
    SPALLOC_UNMARK_BLOCK (page, i + block);
  
  spalloc_upgrade_biggest (page);
  
  spin_unlock (&spalloc_lock);
}

void
spalloc_debug_page (struct spalloc_page *page)
{
  DWORD i;
  
  debug ("*** spalloc_debug_page: %p ***\n", page);
  debug (" sp_pages: %d,", page->sp_pages);
  printk (" sp_biggest: %d,", page->sp_biggest);
  printk (" sp_bigptr: %d\n", page->sp_bigptr);
}

void
spalloc_debug_all ()
{
  struct spalloc_page *this;

  this = spalloc_first;

  while (this)
  {
    spalloc_debug_page (this);

    this = this->sp_next;
  }
}

DEBUG_FUNC (spalloc_find_suitable_page);
DEBUG_FUNC (spalloc_upgrade_biggest);
DEBUG_FUNC (spalloc_new_page);
DEBUG_FUNC (spalloc_page_register);
DEBUG_FUNC (spalloc);
DEBUG_FUNC (spfree);
DEBUG_FUNC (spalloc_debug_page);
DEBUG_FUNC (spalloc_debug_all);

