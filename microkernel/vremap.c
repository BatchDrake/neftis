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
 
#include <types.h>

#include <mm/regions.h>
#include <mm/coloring.h>
#include <mm/vm.h>
#include <mm/vremap.h>
#include <mm/salloc.h>
#include <mm/slab.h>

#include <lock/lock.h>

#include <misc/list.h>
#include <task/loader.h>

#include <arch.h>
#include <kctx.h>

#include <util.h>

static struct kmem_cache *vremap_chunk_cache;

static int
vremap_pagefault (struct task *task, struct vm_region *region, busword_t failed_addr)
{
  error ("vremap region fault: %p not present\n", failed_addr);
 
  return -1;
}

struct vremap_free_chunk *
vremap_free_chunk_new (busword_t base, busword_t pages)
{
  struct vremap_free_chunk *new;
  
  if ((new = kmem_cache_alloc (vremap_chunk_cache)) == NULL)
    return NULL;
  
  new->address = base;
  new->pages   = pages;
  
  return new;
}

void
vremap_free_chunk_destroy (struct vremap_free_chunk *chunk)
{
  kmem_cache_free (vremap_chunk_cache, chunk);
}

struct vremap_data *
vremap_data_new (busword_t base, busword_t pages)
{
  struct vremap_data *new;
  
  CONSTRUCT_STRUCT (vremap_data, new);
  
  new->pages_total = pages;
  new->pages_busy  = 0;
  
  new->address     = base;

  if ((new->chunk_list_head = vremap_free_chunk_new (base, pages)) == NULL)
  {
    sfree (new);

    return NULL;
  }

  return new;
}

void
vremap_data_destroy (struct vremap_data *data)
{
  struct vremap_free_chunk *curr, *next;

  curr = data->chunk_list_head;
  
  while (curr != NULL)
  {
    next = LIST_NEXT (curr);
    
    vremap_free_chunk_destroy (curr);

    curr = next;
  }
  
  sfree (data);
}

int
vremap_destroy (struct task *task, struct vm_region *region)
{
  struct vremap_data *data = (struct vremap_data *) region->vr_ops_data;

  vremap_data_destroy (data);

  return 0;
}

struct vm_region_ops vremap_region_ops =
{
  .name        = "vremap",
  .destroy     = vremap_destroy,
  .read_fault  = vremap_pagefault,
  .write_fault = vremap_pagefault,
  .exec_fault  = vremap_pagefault
};

struct vm_region *
vm_region_vremap_new (busword_t virt, busword_t pages, DWORD perms)
{
  struct vm_region         *new;
  struct vremap_data       *data  = NULL;
  
  if ((data = vremap_data_new (virt, pages)) == NULL)
    goto fail;
  
  if ((new = vm_region_new (virt, virt + (pages << __PAGE_BITS) - 1, &vremap_region_ops, data)) == NULL)
    goto fail;

  new->vr_type = VREGION_TYPE_PAGEMAP;
  
  return new;
  
fail:
  if (data != NULL)
    vremap_data_destroy (data);
    
  return NULL;
}

struct vremap_free_chunk *
vm_region_vremap_find (struct vm_region *region, busword_t pages)
{
  struct vremap_data *data = (struct vremap_data *) region->vr_ops_data;

  if (data->chunk_list_head == NULL ||
      data->chunk_list_head->pages < pages)
    return NULL;

  return data->chunk_list_head;
}

int
vremap_data_try_merge (struct vremap_data *data, busword_t address, busword_t pages)
{
  struct vremap_free_chunk *chunk, *found = NULL;
  
  FOR_EACH (chunk, data->chunk_list_head)
  {
    /* Frontwards merge */
    if (address + (pages << __PAGE_BITS) == chunk->address)
    {
      chunk->address = address;
      chunk->pages  += pages;
      found = chunk;
      
      break;
    }
    /* Backwards merge */
    else if (chunk->address + (chunk->pages << __PAGE_BITS) == address)
    {
      chunk->pages  += pages;
      found = chunk;
      
      break;
    }
    else if (chunk->address == address)
      FAIL ("double free found!\n");
  }

  if (found == NULL)
    return -1;

  rsorted_list_remove_element ((void **) &data->chunk_list_head, found);

  /* Try to merge this chunk with the others */
  if (vremap_data_try_merge (data, found->address, found->pages) == -1)
    /* Merge is not possible: insert as a separate chunk */
    rsorted_list_insert ((void **) &data->chunk_list_head, found, found->pages);
  else
    /* Merge was possible: this standalone chunk can be freed */
    vremap_free_chunk_destroy (found);

  return 0;
}

/* TODO: improve this, this operation should never fail */
int
vm_region_vremap_release (struct vm_region *region, busword_t virt, busword_t pages)
{
  struct vremap_data *data = (struct vremap_data *) region->vr_ops_data;
  int result = 0;
  int i;
  
  struct vremap_free_chunk *chunk;
  
  DECLARE_CRITICAL_SECTION (release);

  CRITICAL_ENTER (release);
  
  if (vremap_data_try_merge (data, virt, pages) == -1)
  {
    if ((chunk = vremap_free_chunk_new (virt, pages)) != NULL)
    {
      rsorted_list_insert ((void **) &data->chunk_list_head, chunk, chunk->pages);

      while (pages--)
	vm_region_unmap_page (region, virt + (pages << __PAGE_BITS));
    }
    else
      result = -1;
  }

  CRITICAL_LEAVE (release);

  return result;
}

busword_t
vm_region_vremap_ensure (struct vm_region *region, busword_t pages)
{
  struct vremap_data *data = (struct vremap_data *) region->vr_ops_data;
  struct vremap_free_chunk *chunk;
  busword_t address;

  DECLARE_CRITICAL_SECTION (ensure);
  
  CRITICAL_ENTER (ensure);
  
  /* Do we have space? */
  if ((chunk = vm_region_vremap_find (region, pages)) == NULL)
    address = -1;
  else
  {
    /* Remove from list */
    rsorted_list_remove_element ((void **) &data->chunk_list_head,
				 chunk);

    /* This will be our base address */
    address           = chunk->address;
    data->pages_busy += pages; /* Now we have fewer */
    
    /* Does it have exactly the size we need */
    if (chunk->pages == pages) /* Free it */
      kmem_cache_free (vremap_chunk_cache, chunk);
    else
    {
      /* Resize it */
      chunk->address += pages << __PAGE_BITS;
      chunk->pages   -= pages;

      /* Insert it directly, no merging needed */
      rsorted_list_insert ((void **) &data->chunk_list_head, chunk, chunk->pages);
    }
  }
  
  CRITICAL_LEAVE (ensure);

  return address;
}

void
vremap_init (void)
{
  MANDATORY ((vremap_chunk_cache = kmem_cache_create ("vremap_free_chunk", sizeof (struct vremap_free_chunk), NULL, NULL)) != NULL);
}

						      
