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

static inline void
bitmap_mark (uint8_t *bitmap, memsize_t objno)
{
  bitmap[objno >> 3] |= 1 << (objno & 7);
}


static inline void
bitmap_unmark (uint8_t *bitmap, memsize_t objno)
{
  bitmap[objno >> 3] &= ~(1 << (objno & 7));
}

static inline int
bitmap_get (const uint8_t *bitmap, memsize_t objno)
{
  return bitmap[objno >> 3] & (1 << (objno & 7));
}

static inline memsize_t
bitmap_search_free (const uint8_t *bitmap, memsize_t size, memsize_t hint)
{
  memsize_t i;

  if (hint > size)
    hint = 0;
  
  for (i = hint; i < size; ++i)
    if (bitmap_get (bitmap, i))
      return i;
  
  for (i = 0; i < hint; ++i)
    if (bitmap_get (bitmap, i))
      return i;

  return -1;
}

static void
kmem_construct (struct kmem_cache *cache, void *buf)
{
  (cache->constructor) (cache, buf);
}

static void
kmem_destruct (struct kmem_cache *cache, void *buf)
{
  (cache->destructor) (cache, buf);
}

struct kmem_cache *
kmem_cache_create (const char *name, busword_t size, void (*constructor) (struct kmem_cache *, void *), void (*destructor) (struct kmem_cache *, void *))
{
  struct kmem_cache *new;
  struct big_slab_header *big;
  
  int is_big;
  int bitmap_size = 0;
  int data_offset = 0;
  int i;
  
  memsize_t cache_pages;

  /* TODO: implementation. */

  /* If cache is small cache: prealloc in first page */
  /* If cache is big, forget about bitmaps */

  /* Save it in kmem_cache_list */
  
  if ((new = page_alloc (1)) == NULL)
    return NULL;

  strncpy (new->name, name, MM_SLAB_NAME_MAX - 1);
  
  new->name[MM_SLAB_NAME_MAX - 1] = 0;
  
  new->alignment      = MM_SLAB_DEFAULT_ALIGN;
  new->object_size    = __ALIGN (size, new->alignment);

  /* Enough data to know wether this cache is big */
  is_big = MM_CACHE_IS_BIG (new);
  
  new->object_count   = is_big ? 1 : __UNITS (MM_SMALL_SLAB_SIZE_MAX, new->object_size);
  new->object_used    = 0;
  
  new->last_allocated = MM_SLAB_NO_HINT;
  new->last_freed     = MM_SLAB_NO_HINT;

  /* This field will be doubled every time the cache needs to grow bigger (applies to small caches only) */
  if (is_big)
    new->pages_per_slab = __UNITS (size + __ALIGN (sizeof (struct big_slab_header), new->alignment), PAGE_SIZE);
  else
    new->pages_per_slab = 1;
                                  
  new->pages_per_slab = __UNITS (size, PAGE_SIZE);
  
  
  new->opaque         = NULL;
  
  new->constructor    = constructor;
  new->destructor     = destructor;

  /* Small cache can be preallocated now */
  if (!is_big)
  {
    new->state          = MM_SLAB_STATE_EMPTY;
    bitmap_size         = __UNITS (new->object_count, 8);
    data_offset         = __ALIGN (sizeof (struct kmem_cache) + bitmap_size, new->alignment);

    new->bitmap         = (void *) new + sizeof (struct kmem_cache);
    new->data           = (void *) new + data_offset;

    /* Clear bitmap */
    memset (new->bitmap, 0, bitmap_size);
    
    /* Preallocate all */
    for (i = 0; i < new->object_count; ++i)
      kmem_construct (new, new->data + i * new->object_size);
  }

  /* Save cache to list */
  list_insert_tail ((void *) &kmem_cache_list, new);
  
  return new;
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

static struct big_slab_header *
kmem_cache_alloc_big (struct kmem_cache *cache)
{
  struct big_slab_header *new;

  if ((new = page_alloc (cache->pages_per_slab)) == NULL)
    return NULL;

  new->header = cache;
  new->next   = NULL;
  new->state  = MM_SLAB_STATE_EMPTY;

  new->data   = (void *) new + __ALIGN (sizeof (struct big_slab_header), cache->alignment);

  kmem_construct (cache, new->data);

  return new;
}

/*
 * SizeOf (header) + SizeOf (bitmap) + k * size  = 4096
 * SizeOf (header) + __UNITS (k, 8)  + k * size  = 4096
 * SizeOf (header) + ceil (k / 8)    + k * size  = 4096
 * ceil (SizeOf (header) + k / 8     + k * size) = 4096
 * SizeOf (header)       + k / 8     + k * size (+1) = 4096
 * SizeOf (header) * 8   + k * (1 + 8 * (size (+1)))   = 8 * 4096
 * k = 8 * (4096 - SizeOf (header)) / (1 + 8 * (size (+1)))
 *
 * Adding that (+1) we ensure we'll never run out of page boundary,
 * however this can be improved.
 */
static struct small_slab_header *
kmem_cache_alloc_small (struct kmem_cache *cache)
{
  struct small_slab_header *new;
  int bitmap_size;
  int k;
  int data_offset;
  memsize_t i;
  
  if ((new = page_alloc (cache->pages_per_slab)) == NULL)
    return NULL;

  new->header = cache;
  new->next   = NULL;
  new->state  = MM_SLAB_STATE_EMPTY;

  /* Because of alignment restrictions, bitmap may be
     bigger than required. What we'll do here is get
     the theoretical max number of objects we can store
     in a slab (alignment 1), get the bitmap size,
     align everything and obtain how many objects we
     can store according to the remaining free space. */

  k = 8 * (PAGE_SIZE - sizeof (struct small_slab_header)) / (1 + 8 * (cache->object_size + 1));
  bitmap_size = __UNITS (k, 8);
  data_offset = __ALIGN (sizeof (struct small_slab_header) + bitmap_size, PAGE_SIZE);
  
  new->object_count = (PAGE_SIZE - data_offset) / cache->object_size;
  new->object_used  = 0;
  
  new->bitmap = (void *) new + sizeof (struct small_slab_header);
  new->data   = (void *) new + data_offset;

  new->last_allocated = MM_SLAB_NO_HINT;
  new->last_freed     = MM_SLAB_NO_HINT;

  new->pages          = cache->pages_per_slab;
  
  /* Initialize everything */
  for (i = 0; i < new->object_count; ++i)
    kmem_construct (cache, new->data + i * cache->object_size);

  /* Clear bitmap */
  memset (new->bitmap, 0, bitmap_size);
  
  /* Double it, next slabs will be bigger */
  cache->pages_per_slab <<= 1;

  return new;
}

static void *
__small_kmem_cache_alloc (struct kmem_cache *cache)
{
  int hint = 0;
  memsize_t block;
  struct small_slab_header *small_slab;

  /* Check whether we have no small slabs */
    
  if (cache->next_free.next_small_free == NULL && cache->next_partial.next_small_partial == NULL)
    if ((small_slab = kmem_cache_alloc_small (cache)) == NULL)
      return NULL; /* No memory to allocate small slab */
    else
      list_insert_head ((void **) &cache->next_free.next_small_free, small_slab);
    
  if ((small_slab = cache->next_free.next_small_free) != NULL)
    list_remove_element ((void **) &cache->next_free.next_small_free, small_slab);
  else if ((small_slab = cache->next_partial.next_small_partial) != NULL)
    list_remove_element ((void **) &cache->next_partial.next_small_partial, small_slab);

  if (small_slab->last_freed != MM_SLAB_NO_HINT)
    hint = small_slab->last_freed;
  else if (small_slab->last_allocated != MM_SLAB_NO_HINT)
    hint = small_slab->last_allocated + 1;
    
  if ((block = bitmap_search_free (small_slab->bitmap, small_slab->object_count, hint)) != -1)
  {
    /* Found usable block */

    bitmap_mark (small_slab->bitmap, block);

    if (++small_slab->object_used == small_slab->object_count)
    {
      small_slab->state = MM_SLAB_STATE_FULL;
      list_insert_head ((void **) &cache->next_full.next_small_full, small_slab);
    }
    else
    {
      small_slab->state = MM_SLAB_STATE_PARTIAL;
      list_insert_head ((void **) &cache->next_partial.next_small_partial, small_slab);
    }
  }

  return OBJECT_ADDR_SLAB (small_slab, block);
}

static void *
__big_kmem_cache_alloc(struct kmem_cache *cache)
{
  struct big_slab_header *big_slab;

  /* Check whether we have no big slabs */
  
  if (cache->next_free.next_big_free == NULL)
    if ((big_slab = kmem_cache_alloc_big (cache)) == NULL)
      return NULL; /* No memory to allocate small slab */
    else
      list_insert_head ((void **) &cache->next_free.next_big_free, big_slab);
    
  /* Big slabs are either free or used */
  list_remove_element ((void **) &cache->next_free.next_big_free, big_slab);

  big_slab->state = MM_SLAB_STATE_FULL;

  /* Move to full list */
  list_insert_head ((void **) &cache->next_full.next_big_full, big_slab);

  /* Big slabs are like small slabs with only one slot */
  return big_slab->data;
}


void *
kmem_cache_alloc (struct kmem_cache *cache)
{
  int hint = 0;
  memsize_t block;
  struct big_slab_header *big_slab;
  
  /* Small slabs: */
  /*   Check if head is partial or empty. If it is, allocate there*/
  
  if (!MM_CACHE_IS_BIG (cache))
  {
    if (cache->state == MM_SLAB_STATE_EMPTY ||
        cache->state == MM_SLAB_STATE_PARTIAL)
    {
      if (cache->last_freed != MM_SLAB_NO_HINT)
        hint = cache->last_freed;
      else if (cache->last_allocated != MM_SLAB_NO_HINT)
        hint = cache->last_allocated + 1;

      if ((block = bitmap_search_free (cache->bitmap, cache->object_count, hint)) != -1)
      {
        /* Found usable block */

        bitmap_mark (cache->bitmap, block);

        if (++cache->object_used == cache->object_count)
          cache->state = MM_SLAB_STATE_FULL;

        return OBJECT_ADDR_CACHE (cache, block);
      }
    }
    else
      return __small_kmem_cache_alloc (cache); /* Regular alloc */
  }
  else
    return __big_kmem_cache_alloc (cache);
  
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
