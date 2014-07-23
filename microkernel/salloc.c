/*
 *    salloc.c: malloc style functions on top of SLAB allocator
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

/* This will be the final replacement for the awful spalloc () */

#include <string.h>
#include <util.h>

#include <mm/regions.h>
#include <mm/slab.h>
#include <mm/salloc.h>

void *
salloc (size_t size)
{
  char name[MM_SLAB_NAME_MAX] = "pool-";
  memsize_t storage = MM_SLAB_DEFAULT_ALIGN;
  struct kmem_cache *cache;
  void *ptr;
  
  while (storage < size)
    storage <<= 1;

  ultostr (storage, name + strlen (name), MM_SLAB_NAME_MAX - strlen (name));

  if ((cache = kmem_cache_lookup (name)) == NULL)
    if ((cache = kmem_cache_create (name, storage, NULL, NULL)) == NULL)
      return NULL;

  if ((ptr = kmem_cache_alloc (cache)) == NULL)
  {
    if (kmem_cache_grow (cache) == -1)
      return NULL;

    ptr = kmem_cache_alloc (cache);
  }
  
  return ptr;
}

void
sfree (void *ptr)
{
  ASSERT (ptr);
  
  struct generic_slab_header *header = (struct generic_slab_header *) PAGE_START (ptr);

  kmem_cache_free (header->header, ptr);
}

DEBUG_FUNC (salloc);
DEBUG_FUNC (sfree);
