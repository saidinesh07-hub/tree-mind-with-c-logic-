/**
 * TreeMind — WebAssembly Bridge
 * wasm_bridge.c
 *
 * Compiled with Emscripten:
 *   emcc bst.c rbt.c bplus.c wasm_bridge.c \
 *        -o treemind.js \
 *        -s EXPORTED_FUNCTIONS="[
 *              '_tm_init','_tm_bst_insert','_tm_bst_search','_tm_bst_delete',
 *              '_tm_rbt_insert','_tm_rbt_search','_tm_rbt_delete',
 *              '_tm_bplus_insert','_tm_bplus_search','_tm_bplus_delete',
 *              '_tm_switch_tree','_tm_clear','_tm_get_json','_tm_get_json_len',
 *              '_malloc','_free']" \
 *        -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap','UTF8ToString','stringToUTF8']" \
 *        -s ALLOW_MEMORY_GROWTH=1 \
 *        -s MODULARIZE=1 \
 *        -s EXPORT_NAME="TreeMindModule" \
 *        -O2
 *
 * The JS bridge allocates a single shared output buffer and calls
 * tm_get_json() after every mutation to retrieve the updated tree + steps
 * as a JSON string.
 */

#include "bst.h"
#include "rbt.h"
#include "bplus.h"
#include "btree.h"
#include "splay.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ── Shared state ──────────────────────────────────────────── */

static BST      *g_bst   = NULL;
static RBT      *g_rbt   = NULL;
static BPlusTree*g_bplus = NULL;
static BTree    *g_btree = NULL;
static SplayTree*g_splay = NULL;

#define JSON_BUF_SZ (1 << 17)   /* 128 KB — plenty for 100+ node trees */
static char g_json_buf[JSON_BUF_SZ];

/* which tree is "active" for serialisation after an op */
typedef enum { TREE_BST=0, TREE_RBT=1, TREE_BPLUS=2, TREE_BTREE=3, TREE_SPLAY=4 } TreeKind;
static TreeKind g_active = TREE_BST;

/* ── Emscripten exports ────────────────────────────────────── */
#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#  define EXPORT EMSCRIPTEN_KEEPALIVE
#else
#  define EXPORT
#endif

/* Initialise all three trees */
EXPORT void tm_init(void) {
    if (g_bst)   bst_destroy(g_bst);
    if (g_rbt)   rbt_destroy(g_rbt);
    if (g_bplus) bplus_destroy(g_bplus);
    if (g_btree) btree_destroy(g_btree);
    if (g_splay) splay_destroy(g_splay);

    g_bst   = bst_create();
    g_rbt   = rbt_create();
    g_bplus = bplus_create(3);
    g_btree = btree_create(3);
    g_splay = splay_create();

    /* Pre-load the same sample values as the JS app */
    int sample[] = {50,30,70,20,40,60,80};
    for (int i = 0; i < 7; i++) {
        bst_insert(g_bst,   sample[i]);
        rbt_insert(g_rbt,   sample[i]);
        splay_insert(g_splay, sample[i]);
    }
    int bp_sample[] = {20,40,60,10,30,50,70};
    for (int i = 0; i < 7; i++)
        bplus_insert(g_bplus, bp_sample[i]);

    int bt_sample[] = {40,20,60,10,30,50,70};
    for (int i = 0; i < 7; i++)
        btree_insert(g_btree, bt_sample[i]);

    g_active = TREE_BST;
    bst_to_json(g_bst, g_json_buf, JSON_BUF_SZ);
}

/* Switch active tree (0=BST, 1=RBT, 2=B+) without mutation */
EXPORT void tm_switch_tree(int kind) {
    g_active = (TreeKind)kind;
    switch (g_active) {
        case TREE_BST:   bst_to_json(g_bst,   g_json_buf, JSON_BUF_SZ); break;
        case TREE_RBT:   rbt_to_json(g_rbt,   g_json_buf, JSON_BUF_SZ); break;
        case TREE_BPLUS: bplus_to_json(g_bplus,g_json_buf, JSON_BUF_SZ); break;
        case TREE_BTREE: btree_to_json(g_btree,g_json_buf, JSON_BUF_SZ); break;
        case TREE_SPLAY: splay_to_json(g_splay,g_json_buf, JSON_BUF_SZ); break;
    }
}

/* Clear all trees */
EXPORT void tm_clear(void) {
    bst_clear(g_bst);
    rbt_clear(g_rbt);
    bplus_clear(g_bplus);
    btree_clear(g_btree);
    splay_clear(g_splay);
    switch (g_active) {
        case TREE_BST:   bst_to_json(g_bst,   g_json_buf, JSON_BUF_SZ); break;
        case TREE_RBT:   rbt_to_json(g_rbt,   g_json_buf, JSON_BUF_SZ); break;
        case TREE_BPLUS: bplus_to_json(g_bplus,g_json_buf, JSON_BUF_SZ); break;
        case TREE_BTREE: btree_to_json(g_btree,g_json_buf, JSON_BUF_SZ); break;
        case TREE_SPLAY: splay_to_json(g_splay,g_json_buf, JSON_BUF_SZ); break;
    }
}

/* ── BST operations ──────────────────────────────────────── */
EXPORT void tm_bst_insert(int v) {
    bst_insert(g_bst, v);
    g_active = TREE_BST;
    bst_to_json(g_bst, g_json_buf, JSON_BUF_SZ);
}
EXPORT void tm_bst_search(int v) {
    bst_search(g_bst, v);
    g_active = TREE_BST;
    bst_to_json(g_bst, g_json_buf, JSON_BUF_SZ);
}
EXPORT void tm_bst_delete(int v) {
    bst_delete(g_bst, v);
    g_active = TREE_BST;
    bst_to_json(g_bst, g_json_buf, JSON_BUF_SZ);
}

/* ── RBT operations ──────────────────────────────────────── */
EXPORT void tm_rbt_insert(int v) {
    rbt_insert(g_rbt, v);
    g_active = TREE_RBT;
    rbt_to_json(g_rbt, g_json_buf, JSON_BUF_SZ);
}
EXPORT void tm_rbt_search(int v) {
    rbt_search(g_rbt, v);
    g_active = TREE_RBT;
    rbt_to_json(g_rbt, g_json_buf, JSON_BUF_SZ);
}
EXPORT void tm_rbt_delete(int v) {
    rbt_delete(g_rbt, v);
    g_active = TREE_RBT;
    rbt_to_json(g_rbt, g_json_buf, JSON_BUF_SZ);
}

/* ── B+ Tree operations ──────────────────────────────────── */
EXPORT void tm_bplus_insert(int v) {
    bplus_insert(g_bplus, v);
    g_active = TREE_BPLUS;
    bplus_to_json(g_bplus, g_json_buf, JSON_BUF_SZ);
}
EXPORT void tm_bplus_search(int v) {
    bplus_search(g_bplus, v);
    g_active = TREE_BPLUS;
    bplus_to_json(g_bplus, g_json_buf, JSON_BUF_SZ);
}
EXPORT void tm_bplus_delete(int v) {
    bplus_delete(g_bplus, v);
    g_active = TREE_BPLUS;
    bplus_to_json(g_bplus, g_json_buf, JSON_BUF_SZ);
}

/* ── B-Tree operations ──────────────────────────────────── */
EXPORT void tm_btree_insert(int v) {
    btree_insert(g_btree, v);
    g_active = TREE_BTREE;
    btree_to_json(g_btree, g_json_buf, JSON_BUF_SZ);
}
EXPORT void tm_btree_search(int v) {
    btree_search(g_btree, v);
    g_active = TREE_BTREE;
    btree_to_json(g_btree, g_json_buf, JSON_BUF_SZ);
}
EXPORT void tm_btree_delete(int v) {
    btree_delete(g_btree, v);
    g_active = TREE_BTREE;
    btree_to_json(g_btree, g_json_buf, JSON_BUF_SZ);
}

/* ── Splay Tree operations ──────────────────────────────── */
EXPORT void tm_splay_insert(int v) {
    splay_insert(g_splay, v);
    g_active = TREE_SPLAY;
    splay_to_json(g_splay, g_json_buf, JSON_BUF_SZ);
}
EXPORT void tm_splay_search(int v) {
    splay_search(g_splay, v);
    g_active = TREE_SPLAY;
    splay_to_json(g_splay, g_json_buf, JSON_BUF_SZ);
}
EXPORT void tm_splay_delete(int v) {
    splay_delete(g_splay, v);
    g_active = TREE_SPLAY;
    splay_to_json(g_splay, g_json_buf, JSON_BUF_SZ);
}

/* ── JSON retrieval ──────────────────────────────────────── */

/* Returns pointer to the internal JSON buffer (null-terminated) */
EXPORT const char *tm_get_json(void) {
    return g_json_buf;
}

/* Returns byte-length so JS can allocate exactly */
EXPORT int tm_get_json_len(void) {
    return (int)strlen(g_json_buf);
}
