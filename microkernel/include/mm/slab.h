/*
 *    SLAB allocator interface.
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

#ifndef _MM_SLAB_H
#define _MM_SLAB_H

#include <types.h>
#include <defines.h>
#include <misc/list.h>

#define MM_SMALL_SLAB_PAGES    1

#define MM_SMALL_SLAB_SIZE_MAX 2048

#define MM_SLAB_STATE_EMPTY    0
#define MM_SLAB_STATE_PARTIAL  1
#define MM_SLAB_STATE_FULL     2

struct kmem_cache
{
  LINKED_LIST;
  
  /* TODO: lock with mutexes! */
  union
  {
    struct small_slab_header *next_small_full;
    struct big_slab_header   *next_big_full;
  }
  next_full;

  union
  {
    struct small_slab_header *next_small_free;
    struct big_slab_header   *next_big_free;
  }
  next_free;

  union
  {
    struct small_slab_header *next_small_partial;
    struct big_slab_header   *next_big_partial;
  }
  next_partial;
  
  int        state;
  char      *name;

  busword_t object_size;
  int       object_count;

  /* Improve allocation performance by giving
     hints about the bitmap status */
  
  int       last_allocated;
  int       last_freed;
  
  int       pages_per_slab;
  
  uint8_t  *bitmap;
  void     *data;
  void     *opaque;
  
  int       alignment;

  
  void (*constructor) (struct kmem_cache *, void *);
  void (*destructor)  (struct kmem_cache *, void *);
};

struct big_slab_header
{
  struct slab *header;
  struct big_slab_header *next;
  
  int   state;

  void *data;
};
  
struct small_slab_header
{
  struct slab *header;
  struct small_slab_header *next;

  int       state;

  int       object_count;
  uint8_t  *bitmap;
  void     *data;

  /* Hints */
  int       last_allocated;
  int       last_freed;
};

struct kmem_cache *kmem_cache_create (const char *, busword_t, void (*) (struct kmem_cache *, void *), void (*) (struct kmem_cache *, void *));

void  kmem_cache_set_opaque (struct kmem_cache *, void *);
void *kmem_cache_get_opaque (struct kmem_cache *);
void *kmem_cache_alloc (struct kmem_cache *);
void  kmem_cache_free (struct kmem_cache *, void *);
int   kmem_cache_grow (struct kmem_cache *, int);
int   kmem_cache_shrink (struct kmem_cache *);
int   kmem_cache_reap (struct kmem_cache *);
int   kmem_cache_destroy (struct kmem_cache *);

#endif /* _MM_SLAB_H */