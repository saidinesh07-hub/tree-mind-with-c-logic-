/**
 * TreeMind — Binary Search Tree Implementation
 * bst.c
 */

#include "bst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ── Helpers ─────────────────────────────────────────────── */

void step_clear(StepLog *log) { log->count = 0; }

void step_add(StepLog *log, const char *action, const char *reason) {
    if (log->count >= MAX_STEPS) return;
    Step *s = &log->items[log->count++];
    snprintf(s->action, MAX_STR, "%s", action);
    snprintf(s->reason, MAX_STR, "%s", reason);
}

static unsigned long next_id(void) {
    static unsigned long counter = 1000;
    return ++counter;
}

static BSTNode *node_new(int val) {
    BSTNode *n = calloc(1, sizeof *n);
    n->val  = val;
    n->id   = next_id();
    snprintf(n->color, 16, "node");
    return n;
}

static void node_free_all(BSTNode *n) {
    if (!n) return;
    node_free_all(n->left);
    node_free_all(n->right);
    free(n);
}

/* ── BST lifecycle ───────────────────────────────────────── */

BST *bst_create(void) {
    BST *t = calloc(1, sizeof *t);
    return t;
}

void bst_destroy(BST *t) {
    if (!t) return;
    node_free_all(t->root);
    free(t);
}

void bst_clear(BST *t) {
    node_free_all(t->root);
    t->root = NULL;
    step_clear(&t->steps);
}

/* ── Insert ─────────────────────────────────────────────── */

void bst_insert(BST *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR], rsn[MAX_STR];

    BSTNode *node = node_new(val);

    if (!t->root) {
        t->root = node;
        snprintf(act, MAX_STR, "Insert %d", val);
        snprintf(rsn, MAX_STR, "Tree is empty — %d becomes the root node.", val);
        step_add(&t->steps, act, rsn);
        return;
    }

    BSTNode *cur = t->root, *par = NULL;
    char dir[8] = "";

    while (cur) {
        snprintf(act, MAX_STR, "Compare %d vs %d", val, cur->val);
        snprintf(rsn, MAX_STR, "%d %s %d → go %s",
                 val,
                 val < cur->val ? "<" : val > cur->val ? ">" : "=",
                 cur->val,
                 val < cur->val ? "LEFT" : "RIGHT");
        step_add(&t->steps, act, rsn);

        par = cur;
        if (val < cur->val) {
            snprintf(dir, 8, "left");
            cur = cur->left;
        } else if (val > cur->val) {
            snprintf(dir, 8, "right");
            cur = cur->right;
        } else {
            snprintf(act, MAX_STR, "Duplicate %d", val);
            step_add(&t->steps, act,
                     "Value already exists — BST does not allow duplicates.");
            free(node);
            return;
        }
    }

    if (strcmp(dir, "left") == 0)  par->left  = node;
    else                            par->right = node;

    snprintf(act, MAX_STR, "Place %d as %s child of %d", val, dir, par->val);
    snprintf(rsn, MAX_STR, "%d %s %d so it goes %s.",
             val,
             strcmp(dir,"left")==0 ? "<" : ">=",
             par->val,
             strcmp(dir,"left")==0 ? "LEFT" : "RIGHT");
    step_add(&t->steps, act, rsn);
}

/* ── Search ─────────────────────────────────────────────── */

void bst_search(BST *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR], rsn[MAX_STR];
    BSTNode *cur = t->root;
    int depth = 0;

    while (cur) {
        snprintf(act, MAX_STR, "Visit node %d", cur->val);
        snprintf(rsn, MAX_STR, "Depth %d: compare %d vs %d", depth, val, cur->val);
        step_add(&t->steps, act, rsn);

        if (val == cur->val) {
            snprintf(act, MAX_STR, "Found %d!", val);
            snprintf(rsn, MAX_STR, "Search complete in %d comparisons.", depth + 1);
            step_add(&t->steps, act, rsn);
            return;
        }
        if (val < cur->val) {
            snprintf(act, MAX_STR, "Go LEFT from %d", cur->val);
            snprintf(rsn, MAX_STR, "%d < %d", val, cur->val);
            step_add(&t->steps, act, rsn);
            cur = cur->left;
        } else {
            snprintf(act, MAX_STR, "Go RIGHT from %d", cur->val);
            snprintf(rsn, MAX_STR, "%d > %d", val, cur->val);
            step_add(&t->steps, act, rsn);
            cur = cur->right;
        }
        depth++;
    }

    snprintf(act, MAX_STR, "%d NOT FOUND", val);
    step_add(&t->steps, act, "Reached null — value does not exist in tree.");
}

/* ── Delete ─────────────────────────────────────────────── */

static BSTNode *_del(BST *t, BSTNode *node, int val) {
    char act[MAX_STR], rsn[MAX_STR];
    if (!node) {
        step_add(&t->steps, "Node not found", "Value does not exist in tree");
        return NULL;
    }
    if (val < node->val) {
        snprintf(act, MAX_STR, "Go LEFT from %d", node->val);
        snprintf(rsn, MAX_STR, "%d < %d", val, node->val);
        step_add(&t->steps, act, rsn);
        node->left = _del(t, node->left, val);
    } else if (val > node->val) {
        snprintf(act, MAX_STR, "Go RIGHT from %d", node->val);
        snprintf(rsn, MAX_STR, "%d > %d", val, node->val);
        step_add(&t->steps, act, rsn);
        node->right = _del(t, node->right, val);
    } else {
        /* found the node */
        if (!node->left && !node->right) {
            snprintf(act, MAX_STR, "Remove leaf %d", val);
            step_add(&t->steps, act, "Leaf node — simply remove.");
            free(node);
            return NULL;
        }
        if (!node->left) {
            snprintf(act, MAX_STR, "Replace %d with right child", val);
            step_add(&t->steps, act, "Only right child exists.");
            BSTNode *tmp = node->right;
            free(node);
            return tmp;
        }
        if (!node->right) {
            snprintf(act, MAX_STR, "Replace %d with left child", val);
            step_add(&t->steps, act, "Only left child exists.");
            BSTNode *tmp = node->left;
            free(node);
            return tmp;
        }
        /* two children: find in-order successor */
        BSTNode *succ = node->right;
        while (succ->left) succ = succ->left;
        snprintf(act, MAX_STR, "Replace %d with in-order successor %d", val, succ->val);
        step_add(&t->steps, act,
                 "Node has two children — use smallest value from right subtree.");
        node->val = succ->val;
        node->right = _del(t, node->right, succ->val);
    }
    return node;
}

void bst_delete(BST *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR];
    snprintf(act, MAX_STR, "Delete %d", val);
    step_add(&t->steps, act, "Finding node to delete...");
    t->root = _del(t, t->root, val);
}

/* ── Serialisation helpers ───────────────────────────────── */

/* Escape JSON strings */
static void json_str(char *out, size_t sz, const char *in) {
    size_t j = 0;
    for (size_t i = 0; in[i] && j + 4 < sz; i++) {
        char c = in[i];
        if      (c == '"')  { out[j++]='\\'; out[j++]='"';  }
        else if (c == '\\') { out[j++]='\\'; out[j++]='\\'; }
        else                  out[j++] = c;
    }
    out[j] = '\0';
}

/* DFS: emit nodes + edges for the visualiser */
static void dfs_json(const BSTNode *n,
                     double x, double y, double xoff,
                     char *buf, size_t bufsz, size_t *pos)
{
    if (!n || *pos + 256 > bufsz) return;

    /* edge to left child */
    if (n->left)
        *pos += (size_t)snprintf(buf + *pos, bufsz - *pos,
            "{\"type\":\"edge\",\"x1\":%.1f,\"y1\":%.1f,\"x2\":%.1f,\"y2\":%.1f},",
            x, y, x - xoff, y + 80.0);
    /* edge to right child */
    if (n->right)
        *pos += (size_t)snprintf(buf + *pos, bufsz - *pos,
            "{\"type\":\"edge\",\"x1\":%.1f,\"y1\":%.1f,\"x2\":%.1f,\"y2\":%.1f},",
            x, y, x + xoff, y + 80.0);

    char color[32]; snprintf(color, 32, "%s", n->color);
    *pos += (size_t)snprintf(buf + *pos, bufsz - *pos,
        "{\"id\":%lu,\"val\":%d,\"x\":%.1f,\"y\":%.1f,\"color\":\"%s\"},",
        n->id, n->val, x, y, color);

    dfs_json(n->left,  x - xoff, y + 80.0, xoff / 1.7, buf, bufsz, pos);
    dfs_json(n->right, x + xoff, y + 80.0, xoff / 1.7, buf, bufsz, pos);
}

void bst_to_json(const BST *t, char *buf, size_t bufsz) {
    size_t pos = 0;
    pos += (size_t)snprintf(buf + pos, bufsz - pos, "{\"nodes\":[");

    if (t->root) {
        dfs_json(t->root, 0.0, 40.0, 110.0, buf, bufsz, &pos);
        /* trim trailing comma */
        if (pos > 0 && buf[pos-1] == ',') buf[--pos] = '\0';
    }

    pos += (size_t)snprintf(buf + pos, bufsz - pos, "],\"steps\":[");

    for (int i = 0; i < t->steps.count; i++) {
        char ea[MAX_STR*2], er[MAX_STR*2];
        json_str(ea, sizeof ea, t->steps.items[i].action);
        json_str(er, sizeof er, t->steps.items[i].reason);
        pos += (size_t)snprintf(buf + pos, bufsz - pos,
            "{\"action\":\"%s\",\"reason\":\"%s\"}%s",
            ea, er, i < t->steps.count - 1 ? "," : "");
    }

    snprintf(buf + pos, bufsz - pos, "]}");
}
