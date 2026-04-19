/**
 * TreeMind — B+ Tree Implementation
 * bplus.c
 *
 * Mirrors the JavaScript BPlusTree class behaviour:
 *   insert → _insertLeaf → split if needed → promote to parent
 *   search → traverse internal nodes → check leaf
 *   delete → log only (same as JS)
 */

#include "bplus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Helpers ─────────────────────────────────────────────── */

static unsigned long bp_next_id(void) {
    static unsigned long counter = 9000;
    return ++counter;
}

static BPNode *bp_node_new(int is_leaf) {
    BPNode *n = calloc(1, sizeof *n);
    n->is_leaf = is_leaf;
    n->id      = bp_next_id();
    return n;
}

static void bp_node_free_all(BPNode *n) {
    if (!n) return;
    if (!n->is_leaf) {
        for (int i = 0; i < n->n_children; i++)
            bp_node_free_all(n->children[i]);
    }
    free(n);
}

/* Insert val into sorted keys array */
static void keys_insert(int *keys, int *n, int val) {
    int i = *n;
    while (i > 0 && keys[i-1] > val) { keys[i] = keys[i-1]; i--; }
    keys[i] = val;
    (*n)++;
}

/* ── Core insert logic ───────────────────────────────────── */

/*
 * We replicate the JS two-phase approach:
 *   Phase 1 (_find_leaf): traverse root→leaf
 *   Phase 2: insert + split if needed
 *
 * Because the JS doesn't propagate splits up properly for multi-level
 * trees (it only handles one level of split), we implement the same
 * simplified behaviour for visual fidelity.
 */

static BPNode *_find_leaf(BPlusTree *t, int val) {
    BPNode *node = t->root;
    char act[MAX_STR], rsn[MAX_STR];
    while (node && !node->is_leaf) {
        int i = 0;
        while (i < node->n_keys && val > node->keys[i]) i++;
        /* build key string for step log */
        char ks[128] = "[";
        for (int k = 0; k < node->n_keys; k++) {
            char tmp[16]; snprintf(tmp, 16, k ? ",%d" : "%d", node->keys[k]);
            strncat(ks, tmp, sizeof ks - strlen(ks) - 1);
        }
        strncat(ks, "]", sizeof ks - strlen(ks) - 1);
        snprintf(act, MAX_STR, "At internal node %s", ks);
        snprintf(rsn, MAX_STR, "%d → go to child[%d]", val, i);
        step_add(&t->steps, act, rsn);

        BPNode *next = (i < node->n_children) ? node->children[i]
                                               : node->children[node->n_children-1];
        node = next;
    }
    return node;
}

static void _insert_leaf(BPlusTree *t, int val) {
    BPNode *leaf = _find_leaf(t, val);
    if (!leaf) return;

    /* insert in sorted order */
    keys_insert(leaf->keys, &leaf->n_keys, val);

    char ks[256] = "[";
    for (int k = 0; k < leaf->n_keys; k++) {
        char tmp[16]; snprintf(tmp, 16, k ? ",%d" : "%d", leaf->keys[k]);
        strncat(ks, tmp, sizeof ks - strlen(ks) - 1);
    }
    strncat(ks, "]", sizeof ks - strlen(ks) - 1);
    char act[MAX_STR], rsn[MAX_STR];
    /* ks already bounded to 127 chars; safe to compose separately */
    char tmp_act[MAX_STR];
    snprintf(tmp_act, MAX_STR, "Insert %d into leaf ", val);
    strncat(tmp_act, ks, MAX_STR - strlen(tmp_act) - 1);
    snprintf(act, MAX_STR, "%s", tmp_act);
    step_add(&t->steps, act, "Placed in sorted position in leaf node.");

    /* split if over capacity */
    if (leaf->n_keys >= t->order) {
        snprintf(act, MAX_STR, "Split leaf — too many keys (%d)", leaf->n_keys);
        snprintf(rsn, MAX_STR, "Max keys = %d. Split into two leaves.", t->order - 1);
        step_add(&t->steps, act, rsn);

        int mid = (leaf->n_keys + 1) / 2;   /* ceil */
        BPNode *new_leaf = bp_node_new(1);

        /* copy upper half to new_leaf */
        new_leaf->n_keys = leaf->n_keys - mid;
        memcpy(new_leaf->keys, leaf->keys + mid, new_leaf->n_keys * sizeof(int));
        leaf->n_keys = mid;

        /* link leaves */
        new_leaf->next_leaf = leaf->next_leaf;
        leaf->next_leaf     = new_leaf;

        /* promote */
        snprintf(act, MAX_STR, "Promote key %d to parent", new_leaf->keys[0]);
        step_add(&t->steps, act,
                 "B+ Tree: copy-up strategy — keep key in leaf too.");

        if (t->root == leaf) {
            /* create new root */
            BPNode *new_root = bp_node_new(0);
            new_root->keys[0]     = new_leaf->keys[0];
            new_root->n_keys      = 1;
            new_root->children[0] = leaf;
            new_root->children[1] = new_leaf;
            new_root->n_children  = 2;
            t->root = new_root;
        }
        /* (deeper propagation omitted — matches JS simplification) */
    }
}

/* ── Public API ──────────────────────────────────────────── */

BPlusTree *bplus_create(int order) {
    if (order < 2 || order > BPLUS_MAX_ORDER) order = 3;
    BPlusTree *t = calloc(1, sizeof *t);
    t->order = order;
    return t;
}

void bplus_destroy(BPlusTree *t) {
    if (!t) return;
    bp_node_free_all(t->root);
    free(t);
}

void bplus_clear(BPlusTree *t) {
    bp_node_free_all(t->root);
    t->root = NULL;
    step_clear(&t->steps);
}

void bplus_insert(BPlusTree *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR], rsn[MAX_STR];
    snprintf(act, MAX_STR, "B+ Insert %d", val);
    snprintf(rsn, MAX_STR, "Order %d B+ Tree — max %d keys per node.",
             t->order, t->order - 1);
    step_add(&t->steps, act, rsn);

    if (!t->root) {
        t->root = bp_node_new(1);
        t->root->keys[0] = val;
        t->root->n_keys  = 1;
        snprintf(act, MAX_STR, "Create root leaf [%d]", val);
        step_add(&t->steps, act,
                 "First insertion — root is a leaf node.");
        return;
    }
    _insert_leaf(t, val);
}

void bplus_search(BPlusTree *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR], rsn[MAX_STR];
    snprintf(act, MAX_STR, "B+ Search %d", val);
    step_add(&t->steps, act,
             "Traverse from root to leaf following key comparisons.");

    BPNode *node = t->root;
    while (node && !node->is_leaf) {
        int i = 0;
        while (i < node->n_keys && val >= node->keys[i]) i++;
        char ks[128] = "[";
        for (int k = 0; k < node->n_keys; k++) {
            char tmp[16]; snprintf(tmp, 16, k ? ",%d" : "%d", node->keys[k]);
            strncat(ks, tmp, sizeof ks - strlen(ks) - 1);
        }
        strncat(ks, "]", sizeof ks - strlen(ks) - 1);
        snprintf(act, MAX_STR, "At internal node %s", ks);
        snprintf(rsn, MAX_STR, "%d → go to child[%d]", val, i);
        step_add(&t->steps, act, rsn);
        node = (i < node->n_children) ? node->children[i]
                                       : node->children[node->n_children-1];
    }

    if (node) {
        int found = 0;
        for (int i = 0; i < node->n_keys; i++)
            if (node->keys[i] == val) { found = 1; break; }
        if (found) {
            snprintf(act, MAX_STR, "Found %d in leaf!", val);
            step_add(&t->steps, act, "B+ Trees store all data in leaves.");
        } else {
            snprintf(act, MAX_STR, "%d NOT FOUND", val);
            step_add(&t->steps, act,
                     "Value not present in any leaf node.");
        }
    }
}

void bplus_delete(BPlusTree *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR];
    snprintf(act, MAX_STR, "Delete %d", val);
    step_add(&t->steps, act, "B+ delete: find leaf, remove key.");
    /* Full deletion with underflow/merge omitted — matches JS behaviour */
}

/* ── Serialisation ───────────────────────────────────────── */

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

/*
 * BFS layout — mirrors BPlusTree.toArray() in JS.
 * Each node emits one entry with multi=true and val="k1|k2|..."
 */
#define BFS_MAX 256

typedef struct { BPNode *node; double x; int level; } BFSEntry;

void bplus_to_json(const BPlusTree *t, char *buf, size_t bufsz) {
    size_t pos = 0;
    pos += (size_t)snprintf(buf + pos, bufsz - pos, "{\"nodes\":[");

    if (!t->root) goto steps;

    /* ----- BFS to collect levels ----- */
    BFSEntry queue[BFS_MAX];
    int qhead = 0, qtail = 0;

    /* We need level grouping to compute y and spacing */
    /* First pass: collect nodes by level */
    typedef struct { BPNode *node; double x; } LNode;
    static LNode levels[16][BFS_MAX];
    static int   level_cnt[16];
    memset(level_cnt, 0, sizeof level_cnt);

    queue[qtail++] = (BFSEntry){ t->root, 0.0, 0 };
    while (qhead < qtail) {
        BFSEntry e = queue[qhead++];
        if (e.level < 16 && level_cnt[e.level] < BFS_MAX)
            levels[e.level][level_cnt[e.level]++] = (LNode){ e.node, e.x };
        if (!e.node->is_leaf) {
            for (int i = 0; i < e.node->n_children; i++) {
                if (e.node->children[i] && qtail < BFS_MAX)
                    queue[qtail++] = (BFSEntry){ e.node->children[i], e.x, e.level+1 };
            }
        }
    }

    /* Second pass: compute x positions per level and emit */
    double y = 40.0;
    for (int li = 0; li < 16 && level_cnt[li] > 0; li++) {
        int cnt = level_cnt[li];
        double spacing = cnt > 1 ? (500.0 / cnt) : 120.0;
        if (spacing < 120.0) spacing = 120.0;

        for (int i = 0; i < cnt; i++) {
            BPNode *n = levels[li][i].node;
            double x  = (i - (cnt - 1) / 2.0) * spacing;

            /* emit edges to children */
            if (!n->is_leaf) {
                int nc = n->n_children;
                for (int ci = 0; ci < nc; ci++) {
                    double cx = (ci - (nc) / 2.0 + 0.5) * spacing + x;
                    pos += (size_t)snprintf(buf + pos, bufsz - pos,
                        "{\"type\":\"edge\",\"x1\":%.1f,\"y1\":%.1f,"
                        "\"x2\":%.1f,\"y2\":%.1f},",
                        x, y, cx, y + 90.0);
                }
            }

            /* build val string "k1|k2|..." */
            char val_str[128] = "";
            for (int k = 0; k < n->n_keys; k++) {
                char tmp[16];
                snprintf(tmp, 16, k ? "|%d" : "%d", n->keys[k]);
                strncat(val_str, tmp, sizeof val_str - strlen(val_str) - 1);
            }

            const char *colorStr = n->is_leaf ? "leaf" : "internal";
            pos += (size_t)snprintf(buf + pos, bufsz - pos,
                "{\"id\":%lu,\"val\":\"%s\",\"x\":%.1f,\"y\":%.1f,"
                "\"color\":\"%s\",\"multi\":true},",
                n->id, val_str, x, y, colorStr);
        }
        y += 90.0;
    }

steps:
    /* trim trailing comma */
    if (pos > 0 && buf[pos-1] == ',') buf[--pos] = '\0';

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
