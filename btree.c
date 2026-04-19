/**
 * TreeMind — B-Tree Implementation
 * btree.c
 */

#include "btree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long bt_next_id(void) {
    static unsigned long counter = 12000;
    return ++counter;
}

static BTreeNode *bt_node_new(int is_leaf) {
    BTreeNode *n = calloc(1, sizeof *n);
    n->is_leaf = is_leaf;
    n->id = bt_next_id();
    return n;
}

static void bt_node_free_all(BTreeNode *n) {
    if (!n) return;
    if (!n->is_leaf) {
        for (int i = 0; i < n->n_children; i++)
            bt_node_free_all(n->children[i]);
    }
    free(n);
}

static void keys_insert(int *keys, int *n, int val) {
    int i = *n;
    while (i > 0 && keys[i - 1] > val) {
        keys[i] = keys[i - 1];
        i--;
    }
    keys[i] = val;
    (*n)++;
}

static void split_child(BTree *t, BTreeNode *parent, int idx, BTreeNode *child) {
    char act[MAX_STR], rsn[MAX_STR];
    BTreeNode *right = bt_node_new(child->is_leaf);
    int mid = child->n_keys / 2;
    int promote = child->keys[mid];

    right->n_keys = child->n_keys - mid - 1;
    for (int i = 0; i < right->n_keys; i++)
        right->keys[i] = child->keys[mid + 1 + i];

    if (!child->is_leaf) {
        right->n_children = right->n_keys + 1;
        for (int i = 0; i < right->n_children; i++)
            right->children[i] = child->children[mid + 1 + i];
    }

    child->n_keys = mid;
    if (!child->is_leaf) child->n_children = child->n_keys + 1;

    for (int i = parent->n_children; i > idx + 1; i--)
        parent->children[i] = parent->children[i - 1];
    parent->children[idx + 1] = right;
    parent->n_children++;

    for (int i = parent->n_keys; i > idx; i--)
        parent->keys[i] = parent->keys[i - 1];
    parent->keys[idx] = promote;
    parent->n_keys++;

    snprintf(act, MAX_STR, "Split node and promote %d", promote);
    snprintf(rsn, MAX_STR, "Node overflow in B-Tree order %d, split around median key.", t->order);
    step_add(&t->steps, act, rsn);
}

static void insert_nonfull(BTree *t, BTreeNode *node, int val) {
    char act[MAX_STR], rsn[MAX_STR];

    if (node->is_leaf) {
        keys_insert(node->keys, &node->n_keys, val);
        snprintf(act, MAX_STR, "Insert %d into leaf node", val);
        step_add(&t->steps, act, "Inserted key in sorted order within leaf.");
        return;
    }

    int i = node->n_keys - 1;
    while (i >= 0 && val < node->keys[i]) i--;
    i++;

    snprintf(act, MAX_STR, "Descend to child[%d]", i);
    snprintf(rsn, MAX_STR, "Choose child interval for %d based on separator keys.", val);
    step_add(&t->steps, act, rsn);

    if (node->children[i]->n_keys >= t->order - 1) {
        split_child(t, node, i, node->children[i]);
        if (val > node->keys[i]) i++;
    }

    insert_nonfull(t, node->children[i], val);
}

BTree *btree_create(int order) {
    if (order < 3 || order > BTREE_MAX_ORDER) order = 3;
    BTree *t = calloc(1, sizeof *t);
    t->order = order;
    return t;
}

void btree_destroy(BTree *t) {
    if (!t) return;
    bt_node_free_all(t->root);
    free(t);
}

void btree_clear(BTree *t) {
    bt_node_free_all(t->root);
    t->root = NULL;
    step_clear(&t->steps);
}

void btree_insert(BTree *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR], rsn[MAX_STR];

    snprintf(act, MAX_STR, "B-Tree Insert %d", val);
    snprintf(rsn, MAX_STR, "Order %d B-Tree with max %d keys per node.", t->order, t->order - 1);
    step_add(&t->steps, act, rsn);

    if (!t->root) {
        t->root = bt_node_new(1);
        t->root->keys[0] = val;
        t->root->n_keys = 1;
        step_add(&t->steps, "Create root leaf", "Tree was empty, created root with first key.");
        return;
    }

    if (t->root->n_keys >= t->order - 1) {
        BTreeNode *new_root = bt_node_new(0);
        new_root->children[0] = t->root;
        new_root->n_children = 1;
        t->root = new_root;
        split_child(t, new_root, 0, new_root->children[0]);
    }

    insert_nonfull(t, t->root, val);
}

void btree_search(BTree *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR], rsn[MAX_STR];

    snprintf(act, MAX_STR, "B-Tree Search %d", val);
    step_add(&t->steps, act, "Traverse internal nodes by key ranges until leaf.");

    BTreeNode *node = t->root;
    while (node) {
        int i = 0;
        while (i < node->n_keys && val > node->keys[i]) i++;

        snprintf(act, MAX_STR, "Visit node with %d keys", node->n_keys);
        snprintf(rsn, MAX_STR, "Compare against separators and choose position %d.", i);
        step_add(&t->steps, act, rsn);

        if (i < node->n_keys && val == node->keys[i]) {
            snprintf(act, MAX_STR, "Found %d", val);
            step_add(&t->steps, act, "Key exists in current node.");
            return;
        }

        if (node->is_leaf) break;
        node = node->children[i];
    }

    snprintf(act, MAX_STR, "%d NOT FOUND", val);
    step_add(&t->steps, act, "Reached leaf without finding the key.");
}

void btree_delete(BTree *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR], rsn[MAX_STR];

    snprintf(act, MAX_STR, "B-Tree Delete %d", val);
    step_add(&t->steps, act, "Simplified delete: remove from leaf when present.");

    BTreeNode *node = t->root;
    while (node) {
        int i = 0;
        while (i < node->n_keys && val > node->keys[i]) i++;

        if (i < node->n_keys && node->keys[i] == val) {
            if (!node->is_leaf) {
                step_add(&t->steps, "Delete not applied", "Full internal-node rebalance is not implemented in this visual mode.");
                return;
            }
            for (int k = i; k < node->n_keys - 1; k++)
                node->keys[k] = node->keys[k + 1];
            node->n_keys--;
            step_add(&t->steps, "Removed key from leaf", "Leaf key removed without rebalancing.");
            return;
        }

        if (node->is_leaf) break;
        snprintf(act, MAX_STR, "Descend to child[%d]", i);
        snprintf(rsn, MAX_STR, "Continue delete search path for %d.", val);
        step_add(&t->steps, act, rsn);
        node = node->children[i];
    }

    snprintf(act, MAX_STR, "%d NOT FOUND", val);
    step_add(&t->steps, act, "Key does not exist in this B-Tree.");
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

#define BFS_MAX 256

typedef struct {
    BTreeNode *node;
    int level;
} QItem;

void btree_to_json(const BTree *t, char *buf, size_t bufsz) {
    size_t pos = 0;
    pos += (size_t)snprintf(buf + pos, bufsz - pos, "{\"nodes\":[");

    if (t->root) {
        QItem q[BFS_MAX];
        int head = 0, tail = 0;
        q[tail++] = (QItem){ t->root, 0 };

        BTreeNode *levels[16][BFS_MAX];
        int level_cnt[16];
        memset(level_cnt, 0, sizeof level_cnt);

        while (head < tail) {
            QItem it = q[head++];
            if (it.level < 16 && level_cnt[it.level] < BFS_MAX)
                levels[it.level][level_cnt[it.level]++] = it.node;
            if (!it.node->is_leaf) {
                for (int i = 0; i < it.node->n_children; i++) {
                    if (it.node->children[i] && tail < BFS_MAX)
                        q[tail++] = (QItem){ it.node->children[i], it.level + 1 };
                }
            }
        }

        double y = 40.0;
        for (int li = 0; li < 16 && level_cnt[li] > 0; li++) {
            int cnt = level_cnt[li];
            double spacing = cnt > 1 ? (520.0 / cnt) : 150.0;
            if (spacing < 120.0) spacing = 120.0;

            for (int i = 0; i < cnt; i++) {
                BTreeNode *n = levels[li][i];
                double x = (i - (cnt - 1) / 2.0) * spacing;

                if (!n->is_leaf) {
                    int nc = n->n_children;
                    for (int ci = 0; ci < nc; ci++) {
                        double cx = (ci - (nc) / 2.0 + 0.5) * spacing + x;
                        pos += (size_t)snprintf(buf + pos, bufsz - pos,
                            "{\"type\":\"edge\",\"x1\":%.1f,\"y1\":%.1f,\"x2\":%.1f,\"y2\":%.1f},",
                            x, y, cx, y + 90.0);
                    }
                }

                char val_str[128] = "";
                for (int k = 0; k < n->n_keys; k++) {
                    char tmp[16];
                    snprintf(tmp, 16, k ? "|%d" : "%d", n->keys[k]);
                    strncat(val_str, tmp, sizeof val_str - strlen(val_str) - 1);
                }

                pos += (size_t)snprintf(buf + pos, bufsz - pos,
                    "{\"id\":%lu,\"val\":\"%s\",\"x\":%.1f,\"y\":%.1f,\"color\":\"%s\",\"multi\":true},",
                    n->id, val_str, x, y, n->is_leaf ? "leaf" : "internal");
            }
            y += 90.0;
        }

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
