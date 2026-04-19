/**
 * TreeMind — Red-Black Tree (Left-Leaning, Sedgewick variant)
 * rbt.h — Public API
 *
 * Implements the Left-Leaning Red-Black Tree (LLRB) which mirrors the
 * original JavaScript RBT implementation exactly:
 *   - Rotate left when right is RED and left is not RED
 *   - Rotate right when left is RED and left->left is RED
 *   - Colour-flip when both children are RED
 */

#ifndef RBT_H
#define RBT_H

#include "bst.h"   /* reuse Step / StepLog */
#include <stddef.h>

#define RED   1
#define BLACK 0

typedef struct RBTNode {
    int            val;
    int            color;      /* RED / BLACK */
    struct RBTNode *left;
    struct RBTNode *right;
    unsigned long  id;
} RBTNode;

typedef struct {
    RBTNode *root;
    StepLog  steps;
} RBT;

RBT  *rbt_create(void);
void  rbt_destroy(RBT *t);
void  rbt_clear(RBT *t);

void  rbt_insert(RBT *t, int val);
void  rbt_search(RBT *t, int val);
void  rbt_delete(RBT *t, int val);

void  rbt_to_json(const RBT *t, char *buf, size_t bufsz);

#endif /* RBT_H */
