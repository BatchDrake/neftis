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
#include <mm/kmalloc.h>

INLINE busword_t 
bytes_to_blocks (size_t bytes)
{
  return bytes / MEMORY_BLOCK_SIZE + !!(bytes % MEMORY_BLOCK_SIZE);
}

INLINE busword_t
blocks_to_bytes (size_t blocks)
{
  return blocks * MEMORY_BLOCK_SIZE;
}

/* get_accurate_chunk: busca el trozo que mejor encaja. */

static struct kas_chunk *
get_accurate_chunk (struct kas_hdr *hdr, size_t size)
{
  struct kas_chunk *best = NULL;
  struct kas_chunk *this;
  int twice;

  for (this = hdr->kh_first, twice = 0; 
       !twice || this != hdr->kh_first; 
       this = this->kc_next)
  {
    if (!twice)
      twice++;

    if (this->kc_size >= size && this->kc_state == KMALLOC_FREE_CHUNK)
    {

      if (best)
      {
        if (best->kc_size > this->kc_size)
          best = this;
      }
      else
        best = this;
    }
  }

  return best;
}


/* get_accurate_chunk: busca el trozo que peor encaja.
   Buena polšªtica si queremos minimizar la fragmentaciš®n externa. */

static struct kas_chunk *
get_worstfit_chunk (struct kas_hdr *hdr, size_t size)
{
  struct kas_chunk *worst = NULL;
  struct kas_chunk *this;
  int twice;

  for (this = hdr->kh_first, twice = 0; 
       !twice || this != hdr->kh_first; 
       this = this->kc_next)
  {
    if (!twice)
      twice++;

    if (this->kc_size >= size && this->kc_state == KMALLOC_FREE_CHUNK)
    {

      if (worst)
      {
        if (worst->kc_size < this->kc_size)
          worst = this;
      }
      else
        worst = this;
    }
  }

  return worst;
}
/* get_chunk: busca el primer trozo que encaja */
static struct kas_chunk *
get_chunk (struct kas_hdr *hdr, size_t size)
{
  struct kas_chunk *this;
  int twice;

  for (this = hdr->kh_first, twice = 0; 
       !twice || this != hdr->kh_first; 
       this = this->kc_next)
  {
    if (!twice)
      twice++;

    if (this->kc_size >= size && this->kc_state == KMALLOC_FREE_CHUNK)
      return this;
  }

  return NULL;
}

INLINE void 
split_chunk (struct kas_hdr *hdr, struct kas_chunk *chunk, size_t where)
{
  busword_t offset;
  struct kas_chunk *new;
  
  offset = blocks_to_bytes (where);

  new = (struct kas_chunk *) ((busword_t) chunk + offset);

  new->kc_canary = KMALLOC_CANARY;
  new->kc_prev   = chunk;
  new->kc_next   = chunk->kc_next;
  chunk->kc_next = new;

  if (chunk == hdr->kh_last)
  {
    hdr->kh_last           = new;
    hdr->kh_first->kc_prev = hdr->kh_last;
    hdr->kh_last->kc_next  = hdr->kh_first;
  }
  
  new->kc_state  = KMALLOC_FREE_CHUNK;
  new->kc_size   = chunk->kc_size - where;
  chunk->kc_size = where;
}

INLINE int 
merge_right_chunk (struct kas_hdr *hdr, struct kas_chunk *chunk)
{
  if (chunk->kc_next == chunk)
    return 0;
  
  if (chunk->kc_next->kc_state == KMALLOC_FREE_CHUNK)
  {
    if (chunk->kc_next == hdr->kh_last)
      hdr->kh_last = chunk;
    
    chunk->kc_size += chunk->kc_next->kc_size;
    chunk->kc_next =  chunk->kc_next->kc_next;
    chunk->kc_next->kc_prev = chunk;

    return 1;
  }

  return 0;
}

INLINE int 
merge_left_chunk (struct kas_hdr *hdr, struct kas_chunk *chunk)
{
  struct kas_chunk *prev;

  prev = chunk->kc_prev;

  if (prev == chunk)
    return 0;
  
  if (prev->kc_state == KMALLOC_FREE_CHUNK)
  {
    if (chunk == hdr->kh_last)
      hdr->kh_last = prev;
    
    prev->kc_size += chunk->kc_size;
    prev->kc_next  = chunk->kc_next;
    prev->kc_next->kc_prev = prev;

    return 1;
  }

  return 0;
}

static void*
__kmalloc_policy (struct kas_hdr *hdr, 
                  size_t size,
                  struct kas_chunk* (*fetcher)(struct kas_hdr *hdr, size_t))
{
  struct kas_chunk *chunk;
  size_t blocks;

  blocks = bytes_to_blocks (size) + 1; /* La cabecera cuenta. */

  spin_lock (&hdr->kh_lock);

  
  if ((chunk = (fetcher) (hdr, blocks)) == NULL)
  {
    spin_unlock (&hdr->kh_lock);
  
    return NULL;
  }

  
  if (chunk->kc_size > blocks) /* Ojo, mirar si el siguiente chunk mide sÃ³lo 1. */
    split_chunk (hdr, chunk, blocks);

  chunk->kc_state = KMALLOC_BUSY_CHUNK;

  spin_unlock (&hdr->kh_lock);
  
  return chunk + 1;
}

void* 
kmalloc (size_t size, struct kas_hdr *hdr)
{
  return __kmalloc_policy (hdr, size, get_worstfit_chunk);
}

void* 
kmalloc_fast (size_t size, struct kas_hdr *hdr)
{
  return __kmalloc_policy (hdr, size, get_chunk);
}

void 
kfree (void *ptr, struct kas_hdr *hdr)
{
  struct kas_chunk *chunk;

  if (ptr == NULL)
    return;
  
  chunk = ptr;
  
  if ((busword_t) --chunk < (busword_t) hdr->kh_start ||
      (busword_t) ptr >= (busword_t) hdr->kh_end)
  {
    /* Esto es un error. */

    printk ("kfree: %p fuera de rango!\n");
    return;
  }

  if (chunk->kc_state != KMALLOC_BUSY_CHUNK)
  {
    /* LiberaciÃ³n doble o corrupciÃ³n. */
    printk ("kfree: liberacion doble / corrupcion (estado %d)\n",
      chunk->kc_state);
    return;
  }

  chunk->kc_state = KMALLOC_FREE_CHUNK;

  if (chunk != hdr->kh_last)
    merge_right_chunk (hdr, chunk);

  if (chunk != hdr->kh_first)
    merge_left_chunk (hdr, chunk);
}

void 
kmalloc_chunk_debug (struct kas_hdr *hdr)
{
  int twice = 0;
  struct kas_chunk *this;

  for (this = hdr->kh_first, twice = 0; 
       !twice || this != hdr->kh_first; 
       this = this->kc_next)
  {
    twice++;
    
    printk ("%c%p(%H)n%pp%p ", this->kc_state ? 'B' : 'F', 
      this, this->kc_size * 16, this->kc_next, this->kc_prev);
  }
  
  putchar ('\n');
}

void 
kmalloc_test (struct kas_hdr *hdr)
{
  void *thing, *thing2, *fst;
  
  kmalloc_chunk_debug (hdr);

  printk ("fst = kmalloc (300)...\n");
  fst = thing = kmalloc (300, hdr);

  printk ("thing2 = kmalloc (300)...\n");
  
  thing2 = kmalloc (300, hdr);

  printk ("thing = kmalloc (300)...\n");
  thing = kmalloc (300, hdr);

  printk ("thing: %p\n", thing);

  kmalloc_chunk_debug (hdr);

  printk ("kfree (thing2); /* thing2 = %p */\n", thing2);
  kfree (thing2, hdr);

  kmalloc_chunk_debug (hdr);

  printk ("kfree (fst);\n");
  
  kfree (fst, hdr);

  kmalloc_chunk_debug (hdr);

  printk ("kfree (thing);\n");
  
  kfree (thing, hdr);

  kmalloc_chunk_debug (hdr);
}

struct kas_hdr *
kmalloc_init (void *start, void *end)
{
  struct kas_hdr *hdr;
  memsize_t size;
  
  hdr = (struct kas_hdr *) start;
  
  
  hdr->kh_magic        = KMALLOC_HEADER_MAGIC;
  hdr->kh_start        = (void *) (hdr + 1);
  hdr->kh_end          = end;
  hdr->kh_first        = (struct kas_chunk *) hdr->kh_start;
  
  hdr->kh_first->kc_canary = KMALLOC_CANARY;
  hdr->kh_first->kc_state  = KMALLOC_FREE_CHUNK;
  
  size = (busword_t) hdr->kh_end -
         (busword_t) hdr->kh_start + 1;
         
  hdr->kh_first->kc_size   = bytes_to_blocks (size);
  hdr->kh_first->kc_next   = hdr->kh_first;
  hdr->kh_first->kc_prev   = hdr->kh_first;

  hdr->kh_last             = hdr->kh_first;
  
  return hdr;
}

