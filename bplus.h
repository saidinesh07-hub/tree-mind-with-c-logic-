/**
 * TreeMind — B+ Tree (order configurable, default 3)
 * bplus.h — Public API
 *
 * Implementation follows the JavaScript BPlusTree class exactly:
 *  - Leaf split uses copy-up (promote lowest key of right leaf to parent)
 *  - BFS-based layout serialisation for multi-key SVG nodes
 */

#ifndef BPLUS_H
#define BPLUS_H

#include "bst.h"   /* reuse Step / StepLog */
#include <stddef.h>

#define BPLUS_MAX_ORDER  8          /* max configurable order */
#define BPLUS_MAX_KEYS   (BPLUS_MAX_ORDER - 1)
#define BPLUS_MAX_CHILDREN (BPLUS_MAX_ORDER + 1)

typedef struct BPNode {
    int            keys[BPLUS_MAX_KEYS + 1]; /* +1 for overflow before split */
    int            n_keys;
    struct BPNode *children[BPLUS_MAX_CHILDREN];
    int            n_children;
    int            is_leaf;
    struct BPNode *next_leaf;  /* leaf linked list */
    unsigned long  id;
} BPNode;

typedef struct {
    BPNode  *root;
    int      order;
    StepLog  steps;
} BPlusTree;

BPlusTree *bplus_create(int order);
void       bplus_destroy(BPlusTree *t);
void       bplus_clear(BPlusTree *t);

void       bplus_insert(BPlusTree *t, int val);
void       bplus_search(BPlusTree *t, int val);
void       bplus_delete(BPlusTree *t, int val);

void       bplus_to_json(const BPlusTree *t, char *buf, size_t bufsz);

#endif /* BPLUS_H */
