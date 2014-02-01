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

/* TODO: NUMA optimizations, detect per-cpu memory ranges and priorize
   searches of contiguous memory in the local bank */
   
#include <string.h>
#include <util.h>

#include <mm/regions.h>
#include <mm/coloring.h>

#include <lock/lock.h>


struct mm_region *mm_regions = NULL;
int               mm_cache_size = 1024 * 1024; /* Parámetro configurable */

/* Warning: kernel should be outside the paging regions */
/* To achieve this, I would be enough to replace the start pointer
   by the kernel end address. */

void
mm_register_region (physptr_t start, physptr_t end)
{
  struct mm_region *new;
  int bitmap_pages;
  int i;

  if (PAGE_OFFSET (start) != 0)
  {
    /* bug! */
    
    return;
  }
  
  new = (struct mm_region *) start;
  
  new->mr_next   = mm_regions;
  
  new->mr_start  = start;
  new->mr_end    = end;
  
  new->mr_size   = (busword_t) end - (busword_t) start + 1;
  new->mr_pages  = __UNITS (new->mr_size, PAGE_SIZE);
  
  new->mr_bitmap = start + PAGE_SIZE; 
  
  spin_unlock (&new->mr_lock);
  bitmap_pages    = MMR_BITMAP_PAGES (new);
  
  /* Bitmap reset */
  memset (new->mr_bitmap, 0, bitmap_pages * PAGE_SIZE);
  
  MMR_MARK_PAGE (new, 0); /* Página de la región */
  
  for (i = 0; i < bitmap_pages; i++)
    MMR_MARK_PAGE (new, 1 + i); /* Páginas del bitmap */

  new->mr_last_freed = 1 + i;
  
  mm_regions = new;
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

/* Lock region before calling this */
busword_t
mm_get_colored_page (struct mm_region *mr, int color)
{
  busword_t pg;
  int avail_colors;
  int first_color;
  
  avail_colors = PAGE_AVAIL_COLORS (mm_cache_size);
  first_color  = ADDR_COLOR (mm_cache_size, mr->mr_start);
  
  /* page_color = (rel_pag + first_color) % avail_colors */
  
  /* (page_color - first_color) % avail_colors = rel_pag */
  
  for (pg = (color - first_color) % avail_colors;
       pg < mr->mr_pages;
       pg += avail_colors)
  {
    if (!MMR_PAGE_STATE (mr, pg))
      return pg;
  }
  
  return (busword_t) KERNEL_ERROR_VALUE;
}

void
do_nothing ()
{
}

busword_t
mm_get_contiguous (struct mm_region *mr, memsize_t pages)
{
  memsize_t i, j, this;
  int avail;
  
  for (i = 0; i < mr->mr_pages; i++)
  {
    this = (i + mr->mr_last_freed) % mr->mr_pages;
    
    if (!MMR_PAGE_STATE (mr, this) && 
      ((this + pages) <= mr->mr_pages))
    {
      avail = 1;
      for (j = 1; j < pages; j++)
        if (MMR_PAGE_STATE (mr, this + j))
        {
          avail = 0;
          break;
        }
      
      if (avail)
        return this;
    }
    
not_yet:

    do_nothing ();
  }
  

  return (busword_t) KERNEL_ERROR_VALUE;
}

INLINE void
mm_mark_range (struct mm_region *mr, memsize_t page, memsize_t pages)
{
  memsize_t i;
  
  for (i = 0; i < pages; i++)
    MMR_MARK_PAGE (mr, page + i);
}

INLINE void
mm_unmark_range (struct mm_region *mr, memsize_t page, memsize_t pages)
{
  memsize_t i;
  
  for (i = 0; i < pages; i++)
    MMR_UNMARK_PAGE (mr, page + i);
}

/* TODO: fix spinlocks */

physptr_t 
page_alloc (memsize_t pages)
{
  struct mm_region *this_region;
  busword_t page;

  this_region = mm_regions;
  
  while (this_region != NULL)
  {
    spin_lock (&this_region->mr_lock);
    
    if ((page = mm_get_contiguous (this_region, pages)) !=
      (busword_t) KERNEL_ERROR_VALUE)
    {
      mm_mark_range (this_region, page, pages);
      
      spin_unlock (&this_region->mr_lock);
      
      return (physptr_t) MMR_PAGE_TO_ADDR (this_region, page);
    }
    
    spin_unlock (&this_region->mr_lock);
  }
  
  return NULL;
}

int
page_free (void *page, memsize_t pages)
{
  struct mm_region *this_region;
  busword_t relpg;
  
  if (PAGE_OFFSET (page) != 0)
  {
    /* bug! */
    
    return -1;
  }
  
  
  this_region = mm_regions;
  
  while (this_region != NULL)
  {
    if (MMR_OWNS_ADDR (this_region, page))
    {
      spin_lock (&this_region->mr_lock);
      
      relpg = MMR_ADDR_TO_PAGE (this_region, page);
      
      mm_unmark_range (this_region, relpg, pages);
      
      spin_unlock (&this_region->mr_lock);
      
      return 0;
    }
    
    this_region = this_region->mr_next;
  }
  
  warning ("invalid range passed: %p (up to %d pages)\n",
    page, pages);
    
  return KERNEL_ERROR_VALUE;
}

DEBUG_FUNC (mm_register_region);
DEBUG_FUNC (mm_set_cache_size);
DEBUG_FUNC (mm_get_cache_size);
DEBUG_FUNC (mm_available_colors);
DEBUG_FUNC (mm_get_colored_page);
DEBUG_FUNC (do_nothing);
DEBUG_FUNC (mm_get_contiguous);
DEBUG_FUNC (mm_mark_range);
DEBUG_FUNC (mm_unmark_range);
DEBUG_FUNC (page_alloc);
DEBUG_FUNC (page_free);

