/**
 * AssetTrace — Trie Implementation
 *
 * Character mapping: a-z -> 0-25, 0-9 -> 26-35, '-' -> 36, '_' -> 37
 * Input is normalized to uppercase internally, stored as lowercase indices.
 */
#include "trie.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ===== Character mapping ===== */

static int char_to_index(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '0' && c <= '9') return 26 + (c - '0');
    if (c == '-') return 36;
    if (c == '_') return 37;
    return -1;  /* Invalid character */
}

static char index_to_char(int idx) {
    if (idx >= 0 && idx < 26) return (char)('A' + idx);  /* Store uppercase */
    if (idx >= 26 && idx < 36) return (char)('0' + (idx - 26));
    if (idx == 36) return '-';
    if (idx == 37) return '_';
    return '?';
}

/* ===== TrieNode helpers ===== */

static TrieNode *trie_node_create(void) {
    TrieNode *node = (TrieNode *)calloc(1, sizeof(TrieNode));
    return node;  /* calloc zeroes everything — children are NULL, counts are 0 */
}

static void trie_node_destroy(TrieNode *node) {
    if (!node) return;
    for (int i = 0; i < TRIE_CHILDREN_COUNT; i++) {
        trie_node_destroy(node->children[i]);
    }
    free(node->asset_ids);
    free(node);
}

static bool trie_node_add_asset(TrieNode *node, int asset_id) {
    /* Check for duplicate */
    for (size_t i = 0; i < node->asset_count; i++) {
        if (node->asset_ids[i] == asset_id) return true;
    }

    if (node->asset_count >= node->asset_capacity) {
        size_t new_cap = node->asset_capacity == 0 ? 2 : node->asset_capacity * 2;
        int *new_ids = (int *)realloc(node->asset_ids, new_cap * sizeof(int));
        if (!new_ids) return false;
        node->asset_ids = new_ids;
        node->asset_capacity = new_cap;
    }

    node->asset_ids[node->asset_count++] = asset_id;
    return true;
}

static bool trie_node_remove_asset(TrieNode *node, int asset_id) {
    for (size_t i = 0; i < node->asset_count; i++) {
        if (node->asset_ids[i] == asset_id) {
            node->asset_ids[i] = node->asset_ids[node->asset_count - 1];
            node->asset_count--;
            if (node->asset_count == 0) {
                node->is_terminal = false;
            }
            return true;
        }
    }
    return false;
}

static bool trie_node_has_children(const TrieNode *node) {
    for (int i = 0; i < TRIE_CHILDREN_COUNT; i++) {
        if (node->children[i]) return true;
    }
    return false;
}

/* ===== TrieResults helpers ===== */

static TrieResults *trie_results_create(void) {
    TrieResults *r = (TrieResults *)malloc(sizeof(TrieResults));
    if (!r) return NULL;
    r->asset_ids = NULL;
    r->serials = NULL;
    r->count = 0;
    r->capacity = 0;
    return r;
}

static bool trie_results_add(TrieResults *r, int asset_id, const char *serial, size_t max_results) {
    if (r->count >= max_results) return false;

    if (r->count >= r->capacity) {
        size_t new_cap = r->capacity == 0 ? 8 : r->capacity * 2;
        if (new_cap > max_results) new_cap = max_results;

        int *new_ids = (int *)realloc(r->asset_ids, new_cap * sizeof(int));
        char **new_serials = (char **)realloc(r->serials, new_cap * sizeof(char *));
        if (!new_ids || !new_serials) {
            if (new_ids) r->asset_ids = new_ids;
            if (new_serials) r->serials = new_serials;
            return false;
        }
        r->asset_ids = new_ids;
        r->serials = new_serials;
        r->capacity = new_cap;
    }

    r->asset_ids[r->count] = asset_id;
    size_t slen = strlen(serial);
    r->serials[r->count] = (char *)malloc(slen + 1);
    if (!r->serials[r->count]) return false;
    memcpy(r->serials[r->count], serial, slen + 1);
    r->count++;
    return true;
}

/* DFS to collect all terminal nodes under a given node */
static void collect_matches(const TrieNode *node, char *buffer, size_t depth,
                            TrieResults *results, size_t max_results) {
    if (!node || results->count >= max_results) return;

    if (node->is_terminal) {
        buffer[depth] = '\0';
        for (size_t i = 0; i < node->asset_count && results->count < max_results; i++) {
            trie_results_add(results, node->asset_ids[i], buffer, max_results);
        }
    }

    for (int i = 0; i < TRIE_CHILDREN_COUNT && results->count < max_results; i++) {
        if (node->children[i]) {
            buffer[depth] = index_to_char(i);
            collect_matches(node->children[i], buffer, depth + 1, results, max_results);
        }
    }
}

/* ===== Public API ===== */

Trie *trie_create(void) {
    Trie *trie = (Trie *)malloc(sizeof(Trie));
    if (!trie) return NULL;

    trie->root = trie_node_create();
    if (!trie->root) {
        free(trie);
        return NULL;
    }

    trie->total_entries = 0;
    return trie;
}

bool trie_insert(Trie *trie, const char *serial, int asset_id) {
    if (!trie || !serial) return false;

    TrieNode *node = trie->root;
    for (size_t i = 0; serial[i] != '\0'; i++) {
        int idx = char_to_index(serial[i]);
        if (idx < 0) continue;  /* Skip invalid characters */

        if (!node->children[idx]) {
            node->children[idx] = trie_node_create();
            if (!node->children[idx]) return false;
        }
        node = node->children[idx];
    }

    bool was_terminal = node->is_terminal;
    node->is_terminal = true;
    if (!trie_node_add_asset(node, asset_id)) return false;

    if (!was_terminal) trie->total_entries++;
    return true;
}

bool trie_search_exact(const Trie *trie, const char *serial,
                       int **out_ids, size_t *out_count) {
    if (!trie || !serial) return false;

    const TrieNode *node = trie->root;
    for (size_t i = 0; serial[i] != '\0'; i++) {
        int idx = char_to_index(serial[i]);
        if (idx < 0) return false;
        if (!node->children[idx]) return false;
        node = node->children[idx];
    }

    if (!node->is_terminal) return false;

    if (out_ids) *out_ids = node->asset_ids;
    if (out_count) *out_count = node->asset_count;
    return true;
}

TrieResults *trie_search_prefix(const Trie *trie, const char *prefix,
                                size_t max_results) {
    TrieResults *results = trie_results_create();
    if (!results) return NULL;

    if (!trie || !prefix) return results;
    if (max_results == 0) max_results = 10;

    /* Navigate to prefix node */
    const TrieNode *node = trie->root;
    size_t prefix_len = strlen(prefix);
    char normalized_prefix[256];

    size_t norm_len = 0;
    for (size_t i = 0; i < prefix_len && i < 255; i++) {
        int idx = char_to_index(prefix[i]);
        if (idx < 0) continue;
        if (!node->children[idx]) return results;  /* Prefix not found */
        node = node->children[idx];
        normalized_prefix[norm_len++] = index_to_char(idx);
    }
    normalized_prefix[norm_len] = '\0';

    /* Collect all terminals under this node using DFS */
    char buffer[512];
    memcpy(buffer, normalized_prefix, norm_len);
    collect_matches(node, buffer, norm_len, results, max_results);

    return results;
}

bool trie_delete(Trie *trie, const char *serial, int asset_id) {
    if (!trie || !serial) return false;

    /* Navigate to the terminal node, collecting path for pruning */
    typedef struct { TrieNode *node; int child_idx; } PathEntry;
    size_t serial_len = strlen(serial);
    PathEntry *path = (PathEntry *)malloc((serial_len + 1) * sizeof(PathEntry));
    if (!path) return false;

    TrieNode *node = trie->root;
    size_t path_len = 0;

    for (size_t i = 0; serial[i] != '\0'; i++) {
        int idx = char_to_index(serial[i]);
        if (idx < 0) continue;
        if (!node->children[idx]) { free(path); return false; }
        path[path_len].node = node;
        path[path_len].child_idx = idx;
        path_len++;
        node = node->children[idx];
    }

    if (!node->is_terminal) { free(path); return false; }

    /* Remove asset ID from terminal */
    if (!trie_node_remove_asset(node, asset_id)) { free(path); return false; }

    /* If terminal still has assets, we're done */
    if (node->is_terminal) { free(path); return true; }

    /* Node is no longer terminal. Prune if it has no children. */
    trie->total_entries--;

    if (!trie_node_has_children(node)) {
        /* Prune upward */
        for (int i = (int)path_len - 1; i >= 0; i--) {
            TrieNode *parent = path[i].node;
            int cidx = path[i].child_idx;
            trie_node_destroy(parent->children[cidx]);
            parent->children[cidx] = NULL;

            /* Stop pruning if parent is terminal or has other children */
            if (parent->is_terminal || trie_node_has_children(parent)) break;
            /* If parent is root, don't destroy it */
            if (i == 0) break;
        }
    }

    free(path);
    return true;
}

void trie_destroy(Trie *trie) {
    if (!trie) return;
    trie_node_destroy(trie->root);
    free(trie);
}

void trie_results_free(TrieResults *results) {
    if (!results) return;
    for (size_t i = 0; i < results->count; i++) {
        free(results->serials[i]);
    }
    free(results->serials);
    free(results->asset_ids);
    free(results);
}
