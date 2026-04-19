/**
 * TreeMind — Red-Black Tree (LLRB) Implementation
 * rbt.c
 *
 * Mirrors the original JS RBT class (Sedgewick left-leaning 2-3 tree):
 *   _rotL, _rotR, _flip — same rotation / flip logic
 */

#include "rbt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Helpers ─────────────────────────────────────────────── */

static unsigned long rbt_next_id(void) {
    static unsigned long counter = 5000;
    return ++counter;
}

static RBTNode *rbt_node_new(int val) {
    RBTNode *n = calloc(1, sizeof *n);
    n->val   = val;
    n->color = RED;
    n->id    = rbt_next_id();
    return n;
}

static void rbt_node_free_all(RBTNode *n) {
    if (!n) return;
    rbt_node_free_all(n->left);
    rbt_node_free_all(n->right);
    free(n);
}

static int is_red(const RBTNode *n) {
    return n && n->color == RED;
}

/* ── Rotations / flip ────────────────────────────────────── */

static RBTNode *rot_left(RBTNode *h) {
    RBTNode *x = h->right;
    h->right = x->left;
    x->left  = h;
    x->color = h->color;
    h->color = RED;
    return x;
}

static RBTNode *rot_right(RBTNode *h) {
    RBTNode *x = h->left;
    h->left  = x->right;
    x->right = h;
    x->color = h->color;
    h->color = RED;
    return x;
}

static void color_flip(RBTNode *h) {
    h->color        = h->color        == RED ? BLACK : RED;
    h->left->color  = h->left->color  == RED ? BLACK : RED;
    h->right->color = h->right->color == RED ? BLACK : RED;
}

/* ── Recursive insert ────────────────────────────────────── */

static RBTNode *_ins(RBT *t, RBTNode *node, int val) {
    char act[MAX_STR], rsn[MAX_STR];

    if (!node) {
        snprintf(act, MAX_STR, "Create RED node %d", val);
        step_add(&t->steps, act,
                 "New nodes start as RED to minimize BLACK-height changes.");
        return rbt_node_new(val);
    }

    if (val < node->val) {
        snprintf(act, MAX_STR, "Go LEFT at %d", node->val);
        snprintf(rsn, MAX_STR, "%d < %d", val, node->val);
        step_add(&t->steps, act, rsn);
        node->left = _ins(t, node->left, val);
    } else if (val > node->val) {
        snprintf(act, MAX_STR, "Go RIGHT at %d", node->val);
        snprintf(rsn, MAX_STR, "%d > %d", val, node->val);
        step_add(&t->steps, act, rsn);
        node->right = _ins(t, node->right, val);
    }
    /* else: duplicate — ignore (same as JS) */

    /* ---- rebalancing (same three checks as JS _ins) ---- */
    if (is_red(node->right) && !is_red(node->left)) {
        snprintf(act, MAX_STR, "Left rotate at %d", node->val);
        step_add(&t->steps, act,
                 "Right RED child, left not RED → rotate left.");
        node = rot_left(node);
    }
    if (is_red(node->left) && is_red(node->left ? node->left->left : NULL)) {
        snprintf(act, MAX_STR, "Right rotate at %d", node->val);
        step_add(&t->steps, act,
                 "Two consecutive LEFT REDs → rotate right.");
        node = rot_right(node);
    }
    if (is_red(node->left) && is_red(node->right)) {
        snprintf(act, MAX_STR, "Color flip at %d", node->val);
        step_add(&t->steps, act,
                 "Both children RED → flip colors to propagate BLACK-height.");
        color_flip(node);
    }
    return node;
}

/* ── Public API ──────────────────────────────────────────── */

RBT *rbt_create(void) { return calloc(1, sizeof(RBT)); }

void rbt_destroy(RBT *t) {
    if (!t) return;
    rbt_node_free_all(t->root);
    free(t);
}

void rbt_clear(RBT *t) {
    rbt_node_free_all(t->root);
    t->root = NULL;
    step_clear(&t->steps);
}

void rbt_insert(RBT *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR];
    snprintf(act, MAX_STR, "RBT Insert %d", val);
    step_add(&t->steps, act,
             "Red-Black Tree insertion — insert as RED, then fix violations.");

    t->root = _ins(t, t->root, val);
    t->root->color = BLACK;
    step_add(&t->steps, "Recolor root BLACK", "Rule: Root is always BLACK.");
}

void rbt_search(RBT *t, int val) {
    /* same traversal as BST */
    step_clear(&t->steps);
    char act[MAX_STR], rsn[MAX_STR];
    RBTNode *cur = t->root;
    int depth = 0;

    while (cur) {
        snprintf(act, MAX_STR, "Visit node %d (%s)", cur->val,
                 cur->color == RED ? "RED" : "BLACK");
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

/*
 * Delete for LLRB is involved; the JS version doesn't implement full
 * RBT delete — it falls through to the BST parent's delete which only
 * logs a "Delete X / finding…" step.  We replicate that exact behaviour
 * so the step log stays identical to the original.
 */
void rbt_delete(RBT *t, int val) {
    step_clear(&t->steps);
    char act[MAX_STR];
    snprintf(act, MAX_STR, "Delete %d", val);
    step_add(&t->steps, act, "Finding node to delete...");
    /* For full correctness a LLRB deletion would go here;
       matching the JS app we only log the intent for now. */
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

static void rbt_dfs_json(const RBTNode *n,
                         double x, double y, double xoff,
                         char *buf, size_t bufsz, size_t *pos)
{
    if (!n || *pos + 256 > bufsz) return;

    if (n->left)
        *pos += (size_t)snprintf(buf + *pos, bufsz - *pos,
            "{\"type\":\"edge\",\"x1\":%.1f,\"y1\":%.1f,\"x2\":%.1f,\"y2\":%.1f},",
            x, y, x - xoff, y + 80.0);
    if (n->right)
        *pos += (size_t)snprintf(buf + *pos, bufsz - *pos,
            "{\"type\":\"edge\",\"x1\":%.1f,\"y1\":%.1f,\"x2\":%.1f,\"y2\":%.1f},",
            x, y, x + xoff, y + 80.0);

    const char *colorStr = (n->color == RED) ? "red" : "black";
    *pos += (size_t)snprintf(buf + *pos, bufsz - *pos,
        "{\"id\":%lu,\"val\":%d,\"x\":%.1f,\"y\":%.1f,\"color\":\"%s\"},",
        n->id, n->val, x, y, colorStr);

    rbt_dfs_json(n->left,  x - xoff, y + 80.0, xoff / 1.7, buf, bufsz, pos);
    rbt_dfs_json(n->right, x + xoff, y + 80.0, xoff / 1.7, buf, bufsz, pos);
}

void rbt_to_json(const RBT *t, char *buf, size_t bufsz) {
    size_t pos = 0;
    pos += (size_t)snprintf(buf + pos, bufsz - pos, "{\"nodes\":[");

    if (t->root) {
        rbt_dfs_json(t->root, 0.0, 40.0, 120.0, buf, bufsz, &pos);
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
