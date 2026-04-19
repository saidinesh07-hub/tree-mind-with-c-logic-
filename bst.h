/**
 * TreeMind — Binary Search Tree (BST)
 * bst.h — Public API
 *
 * All operations store rich step logs that the JS bridge serialises to JSON
 * and feeds into the frontend step-log panel unchanged.
 */

#ifndef BST_H
#define BST_H

#include <stddef.h>

/* ── Step log ─────────────────────────────────────────────── */
#define MAX_STEPS  512
#define MAX_STR    256

typedef struct {
    char action[MAX_STR];
    char reason[MAX_STR];
} Step;

typedef struct {
    Step  items[MAX_STEPS];
    int   count;
} StepLog;

void step_clear(StepLog *log);
void step_add(StepLog *log, const char *action, const char *reason);

/* ── BST node ─────────────────────────────────────────────── */
typedef struct BSTNode {
    int            val;
    struct BSTNode *left;
    struct BSTNode *right;
    char           color[16];   /* "node" for plain BST */
    unsigned long  id;
} BSTNode;

/* ── BST tree ─────────────────────────────────────────────── */
typedef struct {
    BSTNode *root;
    StepLog  steps;
} BST;

/* lifecycle */
BST     *bst_create(void);
void     bst_destroy(BST *t);

/* operations */
void     bst_insert(BST *t, int val);
void     bst_search(BST *t, int val);
void     bst_delete(BST *t, int val);
void     bst_clear(BST *t);

/* serialisation — writes JSON into caller-supplied buffer */
void     bst_to_json(const BST *t, char *buf, size_t bufsz);

#endif /* BST_H */
