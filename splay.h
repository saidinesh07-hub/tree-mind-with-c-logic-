/**
 * TreeMind — Splay Tree
 * splay.h — Public API
 */

#ifndef SPLAY_H
#define SPLAY_H

#include "bst.h"
#include <stddef.h>

typedef struct SplayNode {
    int val;
    struct SplayNode *left;
    struct SplayNode *right;
    unsigned long id;
} SplayNode;

typedef struct {
    SplayNode *root;
    StepLog steps;
} SplayTree;

SplayTree *splay_create(void);
void       splay_destroy(SplayTree *t);
void       splay_clear(SplayTree *t);

void       splay_insert(SplayTree *t, int val);
void       splay_search(SplayTree *t, int val);
void       splay_delete(SplayTree *t, int val);

void       splay_to_json(const SplayTree *t, char *buf, size_t bufsz);

#endif /* SPLAY_H */
