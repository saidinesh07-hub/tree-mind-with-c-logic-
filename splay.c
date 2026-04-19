/**
 * TreeMind — Splay Tree Implementation
 * splay.c
 */

#include "splay.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long sp_next_id(void) {
    static unsigned long counter = 15000;
    return ++counter;
}

static SplayNode *sp_node_new(int val) {
    SplayNode *n = calloc(1, sizeof *n);
    n->val = val;
    n->id = sp_next_id();
    return n;
}

static void sp_node_free_all(SplayNode *n) {
    if (!n) return;
    sp_node_free_all(n->left);
    sp_node_free_all(n->right);
    free(n);
}

static SplayNode *rot_right(SplayNode *x) {
    SplayNode *y = x->left;
    x->left = y->right;
    y->right = x;
    return y;
}

static SplayNode *rot_left(SplayNode *x) {
    SplayNode *y = x->right;
    x->right = y->left;
    y->left = x;
    return y;
}

static SplayNode *splay_step(SplayTree *t, SplayNode *root, int key) {
    char act[MAX_STR], rsn[MAX_STR];

    if (!root || root->val == key) return root;

    if (key < root->val) {
        if (!root->left) return root;

        snprintf(act, MAX_STR, "Splay left from %d", root->val);
        snprintf(rsn, MAX_STR, "%d < %d, move toward left subtree.", key, root->val);
        step_add(&t->steps, act, rsn);

        if (key < root->left->val) {
            step_add(&t->steps, "Zig-Zig step", "Two left moves, rotate right twice.");
            root->left->left = splay_step(t, root->left->left, key);
            root = rot_right(root);
        } else if (key > root->left->val) {
            step_add(&t->steps, "Zig-Zag step", "Left then right, rotate inside subtree first.");
            root->left->right = splay_step(t, root->left->right, key);
            if (root->left->right) root->left = rot_left(root->left);
        }

        return root->left ? rot_right(root) : root;
    }

    if (!root->right) return root;

    snprintf(act, MAX_STR, "Splay right from %d", root->val);
    snprintf(rsn, MAX_STR, "%d > %d, move toward right subtree.", key, root->val);
    step_add(&t->steps, act, rsn);

    if (key > root->right->val) {
        step_add(&t->steps, "Zig-Zig step", "Two right moves, rotate left twice.");
        root->right->right = splay_step(t, root->right->right, key);
        root = rot_left(root);
    } else if (key < root->right->val) {
        step_add(&t->steps, "Zig-Zag step", "Right then left, rotate inside subtree first.");
        root->right->left = splay_step(t, root->right->left, key);
        if (root->right->left) root->right = rot_right(root->right);
    }

    return root->right ? rot_left(root) : root;
}

SplayTree *splay_create(void) {
    return calloc(1, sizeof(SplayTree));
}

void splay_destroy(SplayTree *t) {
    if (!t) return;
    sp_node_free_all(t->root);
    free(t);
}

void splay_clear(SplayTree *t) {
    sp_node_free_all(t->root);
    t->root = NULL;
    step_clear(&t->steps);
}

void splay_insert(SplayTree *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR], rsn[MAX_STR];

    snprintf(act, MAX_STR, "Splay Insert %d", val);
    step_add(&t->steps, act, "Insert then splay key to root for fast repeated access.");

    if (!t->root) {
        t->root = sp_node_new(val);
        step_add(&t->steps, "Create root node", "Tree was empty.");
        return;
    }

    t->root = splay_step(t, t->root, val);

    if (t->root->val == val) {
        step_add(&t->steps, "Duplicate key", "Key already exists, only splay performed.");
        return;
    }

    SplayNode *n = sp_node_new(val);
    if (val < t->root->val) {
        n->right = t->root;
        n->left = t->root->left;
        t->root->left = NULL;
        step_add(&t->steps, "Attach new root", "New key becomes root with previous root on right.");
    } else {
        n->left = t->root;
        n->right = t->root->right;
        t->root->right = NULL;
        step_add(&t->steps, "Attach new root", "New key becomes root with previous root on left.");
    }
    t->root = n;

    snprintf(rsn, MAX_STR, "Inserted %d and moved it to root.", val);
    step_add(&t->steps, "Splay complete", rsn);
}

void splay_search(SplayTree *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR], rsn[MAX_STR];

    snprintf(act, MAX_STR, "Splay Search %d", val);
    step_add(&t->steps, act, "Search path is splayed so accessed key moves near root.");

    if (!t->root) {
        step_add(&t->steps, "Empty tree", "No nodes available.");
        return;
    }

    t->root = splay_step(t, t->root, val);
    if (t->root && t->root->val == val) {
        snprintf(act, MAX_STR, "Found %d", val);
        step_add(&t->steps, act, "Target key is now at root.");
    } else {
        snprintf(act, MAX_STR, "%d NOT FOUND", val);
        step_add(&t->steps, act, "Closest accessed node has been splayed to root.");
    }
}

void splay_delete(SplayTree *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR];

    snprintf(act, MAX_STR, "Splay Delete %d", val);
    step_add(&t->steps, act, "Splay key to root, then remove and join subtrees.");

    if (!t->root) {
        step_add(&t->steps, "Empty tree", "Nothing to delete.");
        return;
    }

    t->root = splay_step(t, t->root, val);
    if (!t->root || t->root->val != val) {
        step_add(&t->steps, "Delete skipped", "Key does not exist after splay.");
        return;
    }

    SplayNode *old_root = t->root;
    if (!t->root->left) {
        t->root = t->root->right;
        step_add(&t->steps, "Remove root", "Left subtree empty, promote right subtree.");
    } else {
        SplayNode *right_sub = t->root->right;
        t->root = splay_step(t, t->root->left, val);
        t->root->right = right_sub;
        step_add(&t->steps, "Join subtrees", "Splayed max of left subtree and attached right subtree.");
    }

    free(old_root);
}

static void json_str(char *out, size_t sz, const char *in) {
    size_t j = 0;
    for (size_t i = 0; in[i] && j + 4 < sz; i++) {
        char c = in[i];
        if (c == '"') { out[j++] = '\\'; out[j++] = '"'; }
        else if (c == '\\') { out[j++] = '\\'; out[j++] = '\\'; }
        else out[j++] = c;
    }
    out[j] = '\0';
}

static void dfs_json(const SplayNode *n,
                     double x, double y, double xoff,
                     char *buf, size_t bufsz, size_t *pos) {
    if (!n || *pos + 256 > bufsz) return;

    if (n->left)
        *pos += (size_t)snprintf(buf + *pos, bufsz - *pos,
            "{\"type\":\"edge\",\"x1\":%.1f,\"y1\":%.1f,\"x2\":%.1f,\"y2\":%.1f},",
            x, y, x - xoff, y + 80.0);
    if (n->right)
        *pos += (size_t)snprintf(buf + *pos, bufsz - *pos,
            "{\"type\":\"edge\",\"x1\":%.1f,\"y1\":%.1f,\"x2\":%.1f,\"y2\":%.1f},",
            x, y, x + xoff, y + 80.0);

    *pos += (size_t)snprintf(buf + *pos, bufsz - *pos,
        "{\"id\":%lu,\"val\":%d,\"x\":%.1f,\"y\":%.1f,\"color\":\"splay\"},",
        n->id, n->val, x, y);

    dfs_json(n->left, x - xoff, y + 80.0, xoff / 1.7, buf, bufsz, pos);
    dfs_json(n->right, x + xoff, y + 80.0, xoff / 1.7, buf, bufsz, pos);
}

void splay_to_json(const SplayTree *t, char *buf, size_t bufsz) {
    size_t pos = 0;
    pos += (size_t)snprintf(buf + pos, bufsz - pos, "{\"nodes\":[");

    if (t->root) {
        dfs_json(t->root, 0.0, 40.0, 110.0, buf, bufsz, &pos);
        if (pos > 0 && buf[pos - 1] == ',') buf[--pos] = '\0';
    }

    pos += (size_t)snprintf(buf + pos, bufsz - pos, "],\"steps\":[");

    for (int i = 0; i < t->steps.count; i++) {
        char ea[MAX_STR * 2], er[MAX_STR * 2];
        json_str(ea, sizeof ea, t->steps.items[i].action);
        json_str(er, sizeof er, t->steps.items[i].reason);
        pos += (size_t)snprintf(buf + pos, bufsz - pos,
            "{\"action\":\"%s\",\"reason\":\"%s\"}%s",
            ea, er, i < t->steps.count - 1 ? "," : "");
    }

    snprintf(buf + pos, bufsz - pos, "]}");
}
