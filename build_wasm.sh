#!/usr/bin/env bash
# =============================================================================
# TreeMind — Emscripten Build Script
# build_wasm.sh
#
# Prerequisites:
#   1. Install Emscripten SDK:
#        git clone https://github.com/emscripten-core/emsdk.git
#        cd emsdk && ./emsdk install latest && ./emsdk activate latest
#        source ./emsdk_env.sh
#
#   2. Run this script from the treemind/ root:
#        chmod +x build_wasm.sh
#        ./build_wasm.sh
#
# Output:
#   dist/treemind.js   — JS glue (import in your HTML)
#   dist/treemind.wasm — compiled binary (serve alongside HTML)
# =============================================================================

set -euo pipefail

SRC="."
OUT="dist"
mkdir -p "$OUT"

EXPORTED_FUNCTIONS='[
  "_tm_init",
  "_tm_switch_tree",
  "_tm_clear",
  "_tm_bst_insert","_tm_bst_search","_tm_bst_delete",
  "_tm_rbt_insert","_tm_rbt_search","_tm_rbt_delete",
  "_tm_bplus_insert","_tm_bplus_search","_tm_bplus_delete",
  "_tm_btree_insert","_tm_btree_search","_tm_btree_delete",
  "_tm_splay_insert","_tm_splay_search","_tm_splay_delete",
  "_tm_get_json","_tm_get_json_len",
  "_malloc","_free"
]'

# Compact to single line (emcc doesn't like newlines in -s strings)
EF=$(echo "$EXPORTED_FUNCTIONS" | tr -d '\n ')

emcc \
  "$SRC/bst.c"          \
  "$SRC/rbt.c"          \
  "$SRC/bplus.c"        \
  "$SRC/btree.c"        \
  "$SRC/splay.c"        \
  "$SRC/wasm_bridge.c"  \
  -o "treemind.js"      \
  -s "EXPORTED_FUNCTIONS=$EF"                                   \
  -s 'EXPORTED_RUNTIME_METHODS=["ccall","cwrap","UTF8ToString"]'\
  -s ALLOW_MEMORY_GROWTH=1                                       \
  -s MODULARIZE=1                                               \
  -s EXPORT_NAME=TreeMindModule                                  \
  -s ENVIRONMENT=web                                             \
  -s SINGLE_FILE=0                                              \
  -O2                                                           \
  --no-entry

cp -f treemind.js "$OUT/treemind.js"
cp -f treemind.wasm "$OUT/treemind.wasm"

echo ""
echo "✅  Build complete!"
echo "   treemind.js        — $(du -h "treemind.js"       | cut -f1)"
echo "   treemind.wasm      — $(du -h "treemind.wasm"     | cut -f1)"
echo "   dist/treemind.js   — $(du -h "$OUT/treemind.js"  | cut -f1)"
echo "   dist/treemind.wasm — $(du -h "$OUT/treemind.wasm"| cut -f1)"
echo ""
echo "Serve the dist/ folder from any static HTTP server and open"
echo "TREEMIND_wasm.html in your browser."
