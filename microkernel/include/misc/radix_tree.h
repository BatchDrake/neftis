#ifndef _RADIX_TREE_H
#define _RADIX_TREE_H

#include <types.h>

#define RADIX_TREE_ROOT_BITS  4
#define RADIX_TREE_BITS       6

#define RADIX_TREE_KEYLEN_MAX 11

#define RADIX_TREE_ROOT_SLOTS (1 << RADIX_TREE_ROOT_BITS)
#define RADIX_TREE_SLOTS      (1 << RADIX_TREE_BITS)

#define RADIX_TREE_NODE_TYPE_DATA 0
#define RADIX_TREE_NODE_TYPE_TREE 1

typedef uint64_t radixkey_t;
typedef uint8_t  radixtag_t;

#define RADIX_TREE_KEYBITS_MAX (sizeof (radixkey_t) << 3)

struct radix_tree_node
{
  radixkey_t key;
  uint8_t    keylen;
  
  union
  {
    struct radix_tree_node *leaves[RADIX_TREE_SLOTS];
    void                   *slots [RADIX_TREE_SLOTS];
  };

  
  uint8_t tags[RADIX_TREE_SLOTS];
};


struct radix_tree_node *radix_tree_node_new (radixkey_t, int);
void **radix_tree_lookup_slot (const struct radix_tree_node *, radixkey_t);
int radix_tree_insert (struct radix_tree_node *, radixkey_t, void *);
void radix_tree_debug (struct radix_tree_node *, unsigned int);

#endif /* _RADIX_TREE_H */
