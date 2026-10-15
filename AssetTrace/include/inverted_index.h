/**
 * AssetTrace — Inverted Index
 *
 * Keyword-to-posting-list index built on the custom HashMap.
 * Each keyword maps to a dynamic array of asset IDs (posting list).
 * Supports AND/OR search modes with relevance ranking.
 *
 * Tokenization: lowercase, strip punctuation, whitespace separation.
 * Title matches are weighted higher than description-only matches.
 *
 * Complexity:
 *   - Tokenization: O(L) where L = input length
 *   - Single keyword lookup: O(1) average via HashMap + O(P) posting traversal
 *   - AND query with K terms: O(K * P_max) for intersection
 *   - OR query with K terms: O(sum of all P_i) for union
 *   - Ranking/sorting: O(R log R) where R = result count (insertion sort for small R)
 */
#ifndef ASSETTRACE_INVERTED_INDEX_H
#define ASSETTRACE_INVERTED_INDEX_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Search mode */
typedef enum {
    SEARCH_AND,
    SEARCH_OR
} SearchMode;

/* Source of a keyword match */
typedef enum {
    MATCH_TITLE       = 1,
    MATCH_DESCRIPTION = 2,
    MATCH_CATEGORY    = 4,
    MATCH_SERIAL      = 8
} MatchSource;

/* A posting entry: an asset ID + match source info */
typedef struct {
    int asset_id;
    int match_sources;   /* Bitmask of MatchSource */
} PostingEntry;

/* A posting list (dynamic array of PostingEntry) */
typedef struct {
    PostingEntry *entries;
    size_t size;
    size_t capacity;
} PostingList;

/* A search result */
typedef struct {
    int asset_id;
    double score;         /* Relevance score */
    int matched_terms;    /* Number of query terms matched */
    int match_sources;    /* Combined bitmask */
} SearchResult;

/* Search results collection */
typedef struct {
    SearchResult *results;
    size_t count;
    size_t capacity;
} SearchResults;

/* Forward declaration — uses HashMap internally */
typedef struct InvertedIndex InvertedIndex;

/* Stop words configuration */
typedef struct {
    const char **words;
    size_t count;
} StopWordList;

/**
 * Create a new inverted index.
 * @param stop_words  Optional stop word list (may be NULL for defaults)
 * @return            New InvertedIndex, or NULL on failure
 */
InvertedIndex *inverted_index_create(const StopWordList *stop_words);

/**
 * Add an asset to the index.
 * Tokenizes title, description, category, and serial.
 * @param asset_id     Unique asset identifier
 * @param title        Asset title (may be NULL)
 * @param description  Asset description (may be NULL)
 * @param category     Asset category (may be NULL)
 * @param serial       Asset serial number (may be NULL)
 * @return             true on success
 */
bool inverted_index_add_asset(InvertedIndex *idx, int asset_id,
                              const char *title, const char *description,
                              const char *category, const char *serial);

/**
 * Search the index.
 * @param query  Search query string
 * @param mode   SEARCH_AND or SEARCH_OR
 * @return       SearchResults (caller must free with search_results_free)
 */
SearchResults *inverted_index_search(const InvertedIndex *idx,
                                     const char *query, SearchMode mode);

/**
 * Remove all entries for an asset.
 * @return true if any entries were removed
 */
bool inverted_index_remove_asset(InvertedIndex *idx, int asset_id);

/**
 * Update an asset's index entries (remove old, add new).
 */
bool inverted_index_update_asset(InvertedIndex *idx, int asset_id,
                                 const char *title, const char *description,
                                 const char *category, const char *serial);

/**
 * Destroy the inverted index and free all memory.
 */
void inverted_index_destroy(InvertedIndex *idx);

/**
 * Free a SearchResults structure.
 */
void search_results_free(SearchResults *results);

/* ===== Tokenization (exposed for testing) ===== */

typedef struct {
    char **tokens;
    size_t count;
    size_t capacity;
} TokenList;

TokenList *tokenize(const char *text);
void token_list_free(TokenList *list);
bool is_stop_word(const InvertedIndex *idx, const char *word);

#ifdef __cplusplus
}
#endif

#endif /* ASSETTRACE_INVERTED_INDEX_H */
