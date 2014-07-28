/*
 *    The brand-new super-fast buddy allocator for Atomik
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

#include <string.h>
#include <util.h>

#include <mm/regions.h>
#include <mm/coloring.h>

#include <lock/lock.h>

struct mm_region *mm_regions = NULL;
int               mm_cache_size = 1024;

static inline unsigned char
mm_region_ceil_order (memsize_t pages)
{
  unsigned char order = 0;

  while (pages > ((busword_t) 1 << order))
    ++order;

  return order;
}

static inline unsigned char
mm_region_floor_order (memsize_t pages)
{
  unsigned char order;
  
  if (((busword_t) 1 << (order = mm_region_ceil_order (pages))) > pages)
    --order;

  return order;
}

void
mm_region_remove_from_bucket (struct mm_region *region, char order, struct page *page)
{ 
  if (page->free_prev == NULL)
    region->mr_buckets[order].first = page->free_next;
  else
    page->free_prev->free_next = page->free_next;

  if (page->free_next != NULL)
    page->free_next->free_prev = page->free_prev;
}

static inline int
__buddies (const struct page *prev, const struct page *next, unsigned char order)
{
  unsigned long buddysize = (busword_t) 1 << order;

  if (prev == NULL || next == NULL)
    return 0;

  return next->index - prev->index == buddysize && !(prev->index & (2 * buddysize - 1));
}

void
mm_region_insert_in_bucket (struct mm_region *region, unsigned char order, struct page *page)
{
  struct page *first, *this, *next;
  int found;
  static int n;
  
  if ((first = region->mr_buckets[order].first) == NULL)
  { 
    region->mr_buckets[order].first = page;
    page->free_prev = NULL;
    page->free_next = NULL;
  }
  else if (page->index < first->index)
  {
    if (__buddies (page, first, order))
    {
      mm_region_remove_from_bucket (region, order, page);
      mm_region_insert_in_bucket (region, order + 1, page);
    }
    else
    {
      page->free_next = first;
      page->free_prev = NULL;
      region->mr_buckets[order].first = page;
    }
  }
  else
  {
    this = first;

    found = 0;
	  
    while (this != NULL)
    {
      next = this->free_next;

      if (next == NULL)
	found = 1;
      else
	found = page->index < next->index;
      
      if (found)
      {
	if (__buddies (this, page, order))
	{
	  mm_region_remove_from_bucket (region, order, this);
	  mm_region_insert_in_bucket (region, order + 1, this);
	}
	else if (__buddies (page, next, order))
	{
	  mm_region_remove_from_bucket (region, order, next);
	  mm_region_insert_in_bucket (region, order + 1, page);
	}
	else
	{
	  if (next != NULL)
	    next->free_prev = page;
	
	  this->free_next = page;

	  page->free_next = next;
	  page->free_prev = this;
	}
	
	break;
      }

      this = next;
    }
  }
}

void
mm_region_free_pages (struct mm_region *region, void *base, memsize_t pages)
{
  busword_t pageno;
  busword_t firstpageno;
  memsize_t order;
  memsize_t howmany;
  
  /* Use bitshifts! */
  pageno      = (busword_t) base >> __PAGE_BITS;
  firstpageno = (busword_t) region->mr_start >> __PAGE_BITS;
  
  ASSERT (pageno >= firstpageno && pageno < (firstpageno + region->mr_pages));
  
  pageno -= firstpageno;

  while (pages)
  {
    order = mm_region_floor_order (pages);

    do
      howmany = (busword_t) 1 << order--;
    while (pageno & (howmany - 1));

    ++order;
    
    mm_region_insert_in_bucket (region, order, &region->mr_page_list[pageno]);
      
    pages  -= howmany;
    pageno += howmany;
  }
}


busword_t
mm_region_alloc_pages (struct mm_region *region, memsize_t pages)
{
  unsigned char order;
  memsize_t fragtrail;
  struct page *page;
  busword_t addr;
  
  order = mm_region_ceil_order (pages);

  /* TODO: check for unaligned buddies */

  do
    page = region->mr_buckets[order++].first; 
  while (order < MM_REGION_BUCKETS && page == NULL);

  if (page == NULL)
    return (busword_t) KERNEL_ERROR_VALUE;

  --order;

  /* Remove the buddy we've found */
  mm_region_remove_from_bucket (region, order, page);

  addr = (busword_t) region->mr_start + (page->index << __PAGE_BITS);

  fragtrail = ((busword_t) 1 << order) - pages;
  
  /* And return the pages we don't need to the allocator */
  if (fragtrail > 0)
    mm_region_free_pages (region, (void *) addr + (pages << __PAGE_BITS), fragtrail);

  return addr;
}

unsigned long
mm_region_debug (struct mm_region *region)
{
  int i;
  struct page *this;
  memsize_t freecount = 0;
  
  debug ("MM Region:\n");
  debug ("  Page count: %d (~%h)\n", (uint32_t) region->mr_pages, (uint32_t) (region->mr_pages << __PAGE_BITS));

  for (i = 0; i < MM_REGION_BUCKETS; ++i)
  {
    if ((this = region->mr_buckets[i].first) != NULL)
    {
      debug ("  Order %d freelist:\n", i);
      
      while (this != NULL)
      {
	debug ("    %w-%w (%lld pages)\n",
		(uint32_t) (this->index << __PAGE_BITS),
		(uint32_t) ((this->index + (((busword_t) 1 << i)) << __PAGE_BITS) - 1), ((busword_t) 1 << i));

	freecount += (busword_t) 1 << i;
	
	this = this->free_next;
      }

      debug ("\n");
    }
  }

  debug ("%d pages free\n", (uint32_t) freecount);

  return freecount;
}

void
mm_register_region (physptr_t start, physptr_t end)
{
  struct mm_region *region;
  memsize_t pages;
  memsize_t metadata_pages;
  unsigned char order;
  memsize_t i;

  pages = __UNITS (end - start + 1, PAGE_SIZE);
  
  region = (struct mm_region *) start;
  
  memset (region, 0, sizeof (struct mm_region));
  
  region->mr_next  = mm_regions;
  region->mr_pages = pages;
  region->mr_start = start;
  region->mr_page_list = (struct page *) (start + __ALIGN (sizeof (struct mm_region), PAGE_SIZE));
  region->mr_end   = end;

  spin_unlock (&region->mr_lock);
  
  metadata_pages   = __UNITS (sizeof (struct mm_region), PAGE_SIZE) + __UNITS (pages * sizeof (struct page), PAGE_SIZE);

  for (i = 0; i < pages; ++i)
  {
    region->mr_page_list[i].index     = i;
    region->mr_page_list[i].refcount  = i < metadata_pages;
    region->mr_page_list[i].free_prev = NULL;
    region->mr_page_list[i].free_next = NULL;
  }

  i = 0;

  /* Initialize free bucket */
  while (pages > 0)
  {
    order = mm_region_floor_order (pages);

    region->mr_buckets[order].first = &region->mr_page_list[i];
	
    i     += (busword_t) 1 << order;
    pages -= (busword_t) 1 << order;
  }

  /* This marks all metadata pages (i.e. page list) as used */
  mm_region_alloc_pages (region, metadata_pages);

  debug ("%W-%W (%H)\n", start, end, end - start + 1);
  
  mm_regions = region;
}

void
mm_set_cache_size (int size)
{
  mm_cache_size = size;
}

int
mm_get_cache_size (void)
{
  return mm_cache_size;
}

int
mm_available_colors (void)
{
  return PAGE_AVAIL_COLORS (mm_cache_size);
}

busword_t
mm_region_alloc_colored_page (struct mm_region *region, int color)
{
  /* This is a stub, page coloring will be implemented later */
  return mm_region_alloc_pages (region, 1);
}

physptr_t 
page_alloc (memsize_t pages)
{
  struct mm_region *this_region;
  busword_t page;
  physptr_t result = NULL;
  
  DECLARE_CRITICAL_SECTION (alloc_section);
  
  CRITICAL_ENTER (alloc_section);
  
  this_region = mm_regions;
  
  if (this_region != NULL)
  {  
    page = mm_region_alloc_pages (this_region, pages);
   
    if (page != (busword_t) KERNEL_ERROR_VALUE)
      result = (physptr_t) page;
  }

  CRITICAL_LEAVE (alloc_section);
  
  return result;
}

int
page_free (void *page, memsize_t pages)
{
  struct mm_region *this_region;
  busword_t relpg;
  int result = 0;
  
  DECLARE_CRITICAL_SECTION (free_section);
  
  if (PAGE_OFFSET (page) != 0)
  {
    /* bug! */
    
    return -1;
  }

  CRITICAL_ENTER (free_section);
  
  this_region = mm_regions;
  
  while (this_region != NULL)
  {
    if (MMR_OWNS_ADDR (this_region, page))
    {
      mm_region_free_pages (this_region, page, pages);
      
      break;
    }
    
    this_region = this_region->mr_next;
  }

  if (this_region == NULL)
  {
    warning ("invalid range passed: %p (up to %d pages)\n",
             page, pages);
    
    result = KERNEL_ERROR_VALUE;
  }

  CRITICAL_LEAVE (free_section);

  return result;
}

DEBUG_FUNC (mm_region_ceil_order);
DEBUG_FUNC (mm_region_floor_order);
DEBUG_FUNC (mm_region_remove_from_bucket);
DEBUG_FUNC (__buddies);
DEBUG_FUNC (mm_region_insert_in_bucket);
DEBUG_FUNC (mm_region_free_pages);
DEBUG_FUNC (mm_region_alloc_pages);
DEBUG_FUNC (mm_region_debug);
DEBUG_FUNC (mm_register_region);
DEBUG_FUNC (mm_set_cache_size);
DEBUG_FUNC (mm_get_cache_size);
DEBUG_FUNC (mm_available_colors);
DEBUG_FUNC (mm_region_alloc_colored_page);
DEBUG_FUNC (page_alloc);
DEBUG_FUNC (page_free);
