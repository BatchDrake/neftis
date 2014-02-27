/*
 *    Atomik radix-tree implementation
 *    Copyright (C) 2014 Gonzalo J. Carracedo <BatchDrake@gmail.com>
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
#include <mm/spalloc.h>

#include <misc/radix_tree.h>

struct radix_tree_node *
radix_tree_node_new (radixkey_t key, int keylen)
{
  struct radix_tree_node *new;

  CONSTRUCT_STRUCT (radix_tree_node, new);

  new->key = key;
  new->keylen = keylen;
  
  return new;
}

struct radix_tree_node *
radix_tree_node_dup (const struct radix_tree_node *node)
{
  struct radix_tree_node *new;

  CONSTRUCT_STRUCT (radix_tree_node, new);

  memcpy (new, node, sizeof (struct radix_tree_node));

  return new;
}

INLINE unsigned int
radix_tree_get_shift (unsigned len)
{
  unsigned int bits = RADIX_TREE_KEYBITS_MAX;

  ASSERT (len <= RADIX_TREE_KEYLEN_MAX);
  
  if (len > 0)
    bits -= RADIX_TREE_ROOT_BITS + RADIX_TREE_BITS * (len - 1);

  return bits;
}

INLINE radixkey_t
radix_key_letter_at (radixkey_t key, unsigned int off)
{
  unsigned int bits = radix_tree_get_shift (off + 1);
  
  return (key >> bits) & ((1 << RADIX_TREE_BITS) - 1);
}

INLINE radixkey_t
radix_key (radixkey_t key, unsigned int len)
{
  unsigned int bits = radix_tree_get_shift (len);
  
  return (key >> bits) << bits;
}

INLINE BOOL
radix_key_equals (radixkey_t a, radixkey_t b, unsigned int len)
{
  return radix_key (a, len) == radix_key (b, len);
}

INLINE unsigned int
radix_key_equal_size (radixkey_t a, radixkey_t b, unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; ++i)
    if (!radix_key_equals (a, b, i + 1))
      break;

  return i;
}

INLINE void **
__radix_tree_node_lookup_slot (const struct radix_tree_node *node, radixkey_t key, unsigned int offset)
{
  radixkey_t sub;
  unsigned int likeness;
  unsigned int acclen;
  
  acclen = offset + node->keylen;

  ASSERT (acclen <= RADIX_TREE_KEYLEN_MAX);

  /* Key doesn't fit completely, need to break this node */
  likeness = radix_key_equal_size (key, node->key, acclen);

  /* First different letter */
  sub = radix_key_letter_at (key, likeness);
  
  ASSERT (likeness <= acclen);

  if (likeness < acclen)
    return KERNEL_INVALID_POINTER;
  
  if (acclen < RADIX_TREE_KEYLEN_MAX - 1)
  {
    if (node->leaves[sub] == NULL)
      return KERNEL_INVALID_POINTER;
    
    return __radix_tree_node_lookup_slot (node->leaves[sub], key, acclen);
  }
 
  return (void **) &node->slots[sub];
}

INLINE radixtag_t *
__radix_tree_node_lookup_tag (const struct radix_tree_node *node, radixkey_t key, unsigned int offset)
{
  radixkey_t sub;
  unsigned int likeness;
  unsigned int acclen;
  
  acclen = offset + node->keylen;

  ASSERT (acclen <= RADIX_TREE_KEYLEN_MAX);

  /* Key doesn't fit completely, need to break this node */
  likeness = radix_key_equal_size (key, node->key, acclen);

  /* First different letter */
  sub = radix_key_letter_at (key, likeness);
  
  ASSERT (likeness <= acclen);

  if (likeness < acclen)
    return KERNEL_INVALID_POINTER;
  
  if (acclen < RADIX_TREE_KEYLEN_MAX - 1)
  {
    if (node->leaves[sub] == NULL)
      return KERNEL_INVALID_POINTER;
    
    return __radix_tree_node_lookup_tag (node->leaves[sub], key, acclen);
  }
 
  return (radixtag_t *) &node->tags[sub];
}


void **
radix_tree_lookup_slot (const struct radix_tree_node *root, radixkey_t key)
{
  return __radix_tree_node_lookup_slot (root, key, 0);
}

radixtag_t *
radix_tree_lookup_tag (const struct radix_tree_node *root, radixkey_t key)
{
  return __radix_tree_node_lookup_tag (root, key, 0);
}

int
radix_tree_set_tag (const struct radix_tree_node *root, radixkey_t key, radixtag_t ntag)
{
  radixtag_t *tag;

  if ((tag = radix_tree_lookup_tag (root, key)) == NULL)
    return -1;

  *tag = ntag;

  return 0;
}

int
radix_tree_add_tags (const struct radix_tree_node *root, radixkey_t key, radixtag_t ntag)
{
  radixtag_t *tag;

  if ((tag = radix_tree_lookup_tag (root, key)) == NULL)
    return -1;

  *tag |= ntag;

  return 0;
}

int
radix_tree_remove_tags (const struct radix_tree_node *root, radixkey_t key, radixtag_t ntag)
{
  radixtag_t *tag;

  if ((tag = radix_tree_lookup_tag (root, key)) == NULL)
    return -1;

  *tag &= ~ntag;

  return 0;
}

INLINE int
__radix_tree_break (struct radix_tree_node *node, unsigned int offset, unsigned int equalsize)
{
  struct radix_tree_node *new_node;
  unsigned int parent_keylen = equalsize - offset;
  unsigned int child_keylen  = node->keylen - parent_keylen;
  radixkey_t sub;

  /*                                offset
                    equalsize         |
     _________________________________|______
    |                                 |      |     
    aaaaaa bbbbbb cccccc dddddd eeeeee ffffff gggggg hhhhhh
    |_______________________________________________|
                   offset + node->keylen
  */
  
  /* To break a node, we do the following:
     - Create a new_node
     - Copy everything from node to new_node
     - Clear node
     - Set new_node as child */

  if ((new_node = radix_tree_node_dup (node)) == KERNEL_INVALID_POINTER)
    return KERNEL_ERROR_VALUE;

  memset (node, 0, sizeof (struct radix_tree_node));

  node->key    = new_node->key;
  node->keylen = parent_keylen;
  new_node->keylen = child_keylen;

  sub = radix_key_letter_at (new_node->key, equalsize);

  node->leaves[sub] = new_node;
  
  return KERNEL_SUCCESS_VALUE;
}

INLINE int
__radix_tree_node_insert (struct radix_tree_node *node, radixkey_t key, void *data, unsigned int offset)
{
  radixkey_t sub;
  unsigned int likeness;
  unsigned int acclen;
  
  struct radix_tree_node *new_node;

  acclen = offset + node->keylen;

  ASSERT (acclen <= RADIX_TREE_KEYLEN_MAX);

  /* Key doesn't fit completely, need to break this node */
  likeness = radix_key_equal_size (key, node->key, acclen);

  /* First different letter */
  sub = radix_key_letter_at (key, likeness);
  
  ASSERT (likeness <= acclen);

  if (likeness < acclen)
  {
    if (__radix_tree_break (node, offset, likeness) == KERNEL_ERROR_VALUE)
      return KERNEL_ERROR_VALUE;

    if ((new_node = radix_tree_node_new (key, RADIX_TREE_KEYLEN_MAX - likeness - 1)) == KERNEL_INVALID_POINTER)
        return KERNEL_ERROR_VALUE;

    node->leaves[sub] = new_node;
    
    node = new_node;
    
    acclen = RADIX_TREE_KEYLEN_MAX - 1;

    sub = radix_key_letter_at (key, acclen);
  }
  
  if (acclen < RADIX_TREE_KEYLEN_MAX - 1)
  {
    if (node->leaves[sub] == NULL)
    {
      if ((new_node = radix_tree_node_new (key, RADIX_TREE_KEYLEN_MAX - likeness - 1)) == KERNEL_INVALID_POINTER)
        return KERNEL_ERROR_VALUE;

      node->leaves[sub] = new_node;
    }
    
    return __radix_tree_node_insert (node->leaves[sub], key, data, acclen);
  }
  else
    node->slots[sub] = data;
  
  return KERNEL_SUCCESS_VALUE;
}

int
radix_tree_insert (struct radix_tree_node *root, radixkey_t key, void *data)
{
  return __radix_tree_node_insert (root, key, data, 0);
}

int
radix_tree_set (struct radix_tree_node **root, radixkey_t key, void *data)
{
  void **ptr;

  if (*root == NULL)
  {
    if ((*root = radix_tree_node_new (key, RADIX_TREE_KEYLEN_MAX - 1)) == NULL)
      return -1;
    
    (*root)->slots[0] = data;

    return 0;
  }
  else
    return radix_tree_insert (*root, key, data);
}

void
radix_tree_destroy (struct radix_tree_node *tree)
{
  warning ("unimplemented!\n");
}


INLINE int
__radix_tree_walk (struct radix_tree_node *node, int (*callback) (radixkey_t, void **, radixtag_t *, void *), void *data, unsigned int offset)
{
  unsigned int acclen;
  int i;
  radixkey_t slotkey;

  if (node == NULL)
    return 0;
  
  acclen = offset + node->keylen;

  if (acclen == RADIX_TREE_KEYLEN_MAX)
    return (callback) (node->key, &node->slots[0], &node->tags[0], data);
  else
    for (i = 0; i < RADIX_TREE_SLOTS; ++i)
      if (node->slots[i] != NULL)
      {
        if (acclen == RADIX_TREE_KEYLEN_MAX - 1)
        {
          slotkey = radix_key (node->key, acclen) | i;
          
          RETURN_ON_FAILURE ((callback) (slotkey, &node->slots[i], &node->tags[i], data));
        }
        else
          RETURN_ON_FAILURE (__radix_tree_walk (node->leaves[i], callback, data, acclen));
      }
}

int
radix_tree_walk (struct radix_tree_node *node, int (*callback) (radixkey_t, void **, radixtag_t *, void *), void *data)
{
  return __radix_tree_walk (node, callback, data, 0);
}

void
radix_tree_debug (struct radix_tree_node *node, unsigned int offset)
{
  unsigned int acclen;
  int i, n;

  acclen = offset + node->keylen;

  for (n = 0; n < offset; ++n)
    putchar (' ');
  
  printk ("(%p,%d) {\n", (uint32_t) node->key, node->keylen);

  if (acclen == RADIX_TREE_KEYLEN_MAX)
  {
    for (n = 0; n < acclen; ++n) putchar (' ');
    
    printk ("*%p\n", node->slots[0]);
  }
  else
  {
    n = 0;
    
    for (i = 0; i < RADIX_TREE_SLOTS; ++i)
    {
      if (node->slots[i] != NULL)
      {
        if (acclen == RADIX_TREE_KEYLEN_MAX - 1)
        {
          for (n = 0; n < acclen; ++n) putchar (' ');
          printk ("[%d] %p - %p\n", i, (uint32_t) node->key, node->slots[i]);
        }
        else
          radix_tree_debug (node->leaves[i], acclen);
      }
    }
  }

  for (n = 0; n < offset; ++n)
    putchar (' ');
  
  printk ("}\n");
}


DEBUG_FUNC (radix_tree_node_new);
DEBUG_FUNC (radix_tree_node_dup);
DEBUG_FUNC (radix_tree_get_shift);
DEBUG_FUNC (radix_key_letter_at);
DEBUG_FUNC (radix_key);
DEBUG_FUNC (radix_key_equals);
DEBUG_FUNC (radix_key_equal_size);
DEBUG_FUNC (__radix_tree_node_lookup_slot);
DEBUG_FUNC (__radix_tree_node_lookup_tag);
DEBUG_FUNC (radix_tree_lookup_slot);
DEBUG_FUNC (radix_tree_lookup_tag);
DEBUG_FUNC (radix_tree_set_tag);
DEBUG_FUNC (radix_tree_add_tags);
DEBUG_FUNC (radix_tree_remove_tags);
DEBUG_FUNC (__radix_tree_break);
DEBUG_FUNC (__radix_tree_node_insert);
DEBUG_FUNC (radix_tree_insert);
DEBUG_FUNC (radix_tree_set);
DEBUG_FUNC (radix_tree_destroy);
DEBUG_FUNC (__radix_tree_walk);
DEBUG_FUNC (radix_tree_walk);
DEBUG_FUNC (radix_tree_debug);
