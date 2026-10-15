/**
 * AssetTrace — Trie for Serial Number Autocomplete
 *
 * Supports alphanumeric characters plus hyphens and underscores.
 * Each terminal node stores associated asset IDs.
 *
 * Complexity:
 *   - Insert: O(L) where L = serial length
 *   - Exact search: O(L)
 *   - Prefix search: O(L + K) where L = prefix length, K = nodes explored
 *   - Delete: O(L)
 */
#ifndef ASSETTRACE_TRIE_H
#define ASSETTRACE_TRIE_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum children per node: 26 letters + 10 digits + hyphen + underscore = 38 */
#define TRIE_CHILDREN_COUNT 38

/* Trie prefix search results */
typedef struct {
    int *asset_ids;
    char **serials;       /* Matched serial numbers */
    size_t count;
    size_t capacity;
} TrieResults;

typedef struct TrieNode {
    struct TrieNode *children[TRIE_CHILDREN_COUNT];
    bool is_terminal;
    int *asset_ids;       /* Dynamic array of asset IDs at terminal */
    size_t asset_count;
    size_t asset_capacity;
} TrieNode;

typedef struct {
    TrieNode *root;
    size_t total_entries;  /* Number of terminal nodes */
} Trie;

/**
 * Create a new trie.
 * @return Pointer to new Trie, or NULL on failure
 */
Trie *trie_create(void);

/**
 * Insert a serial number with its associated asset ID.
 * Serial is normalized (uppercased) before insertion.
 * @return true on success
 */
bool trie_insert(Trie *trie, const char *serial, int asset_id);

/**
 * Search for an exact serial number.
 * @param out_ids  If non-NULL and found, receives pointer to asset IDs array
 * @param out_count If non-NULL and found, receives count of asset IDs
 * @return         true if exact match found
 */
bool trie_search_exact(const Trie *trie, const char *serial,
                       int **out_ids, size_t *out_count);

/**
 * Search for all serials matching a prefix.
 * Results are bounded by max_results.
 * @param prefix       The prefix to search for
 * @param max_results  Maximum number of results to return
 * @return             TrieResults (caller must free with trie_results_free)
 */
TrieResults *trie_search_prefix(const Trie *trie, const char *prefix,
                                size_t max_results);

/**
 * Delete a serial-asset_id association.
 * If the asset_id is the last one at a terminal, the terminal marker is removed
 * and unnecessary nodes are pruned.
 * @return true if something was deleted
 */
bool trie_delete(Trie *trie, const char *serial, int asset_id);

/**
 * Destroy the trie and free all memory.
 */
void trie_destroy(Trie *trie);

/**
 * Free TrieResults.
 */
void trie_results_free(TrieResults *results);

#ifdef __cplusplus
}
#endif

#endif /* ASSETTRACE_TRIE_H */
