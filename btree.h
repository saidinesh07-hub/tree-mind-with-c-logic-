/**
 * TreeMind — B-Tree (order configurable, default 3)
 * btree.h — Public API
 */

#ifndef BTREE_H
#define BTREE_H

#include "bst.h"
#include <stddef.h>

#define BTREE_MAX_ORDER    8
#define BTREE_MAX_KEYS     (BTREE_MAX_ORDER - 1)
#define BTREE_MAX_CHILDREN (BTREE_MAX_ORDER)

typedef struct BTreeNode {
    int keys[BTREE_MAX_KEYS + 1];
    int n_keys;
    struct BTreeNode *children[BTREE_MAX_CHILDREN + 1];
    int n_children;
    int is_leaf;
    unsigned long id;
} BTreeNode;

typedef struct {
    BTreeNode *root;
    int order;
    StepLog steps;
} BTree;

BTree *btree_create(int order);
void   btree_destroy(BTree *t);
void   btree_clear(BTree *t);

void   btree_insert(BTree *t, int val);
void   btree_search(BTree *t, int val);
void   btree_delete(BTree *t, int val);

void   btree_to_json(const BTree *t, char *buf, size_t bufsz);

#endif /* BTREE_H */
