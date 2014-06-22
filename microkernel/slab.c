/*
 *    SLAB allocator implementation.
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

/* TODO: lock! */
struct kmem_cache *kmem_cache_list;

struct kmem_cache *
kmem_cache_create (const char *name, busword_t size, void (*constructor) (struct kmem_cache *, void *), void (*destructor) (struct kmem_cache *, void *))
{
  /* TODO: implementation. */

  /* If cache is small cache: prealloc in first page */
  /* If cache is big, forget about bitmaps */

  /* Save it in kmem_cache_list */
  return NULL;
}

void
kmem_cache_set_opaque (struct kmem_cache *cache, void *opaque)
{
  cache->opaque = opaque;
}

void *
kmem_cache_get_opaque (struct kmem_cache *cache)
{
  return cache->opaque;
}

void *
kmem_cache_alloc (struct kmem_cache *cache)
{
  /* Small slabs: */
  /*   Check if head is partial or empty. If it is, allocate there*/
  /* For both slabs: */
  /*   If it isn't, look first in free list and then in partial */
  /*   If both lists are empty, grow cache */

  /*   If the slab was empty, mark it as partial and move it to partial list */
  /*   If the slab was partial and now is full, mark it as full and move it to full list */

  return NULL;
}

void
kmem_cache_free (struct kmem_cache *cache, void *ptr)
{
  /* Get slab header, free object and check its new state. Move to a new list accordingly */
}

/* Grow cache */
int
kmem_cache_grow (struct kmem_cache *cache, int slabs)
{
  /* Alloc pages and move slab to free list */
}

/* Deliver some unused pages to the kernel */
int
kmem_cache_shrink (struct kmem_cache *cache)
{
  /* Free all slabs in free list */
}

/* Reap free slabs from all caches */
int
kmem_cache_reap (struct kmem_cache *cache)
{
  /* Iterate through kmem_cache_list and move free lists */
}

/* Destroy a cache. Will work only if cache has no partial or used nodes */
int
kmem_cache_destroy (struct kmem_cache *cache)
{
  /* Check if both partial and full are NULL and free everything */
}
