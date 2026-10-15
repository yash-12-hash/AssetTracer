/**
 * AssetTrace — Asset Service Layer
 *
 * Orchestrates DSA structures, database, and business logic.
 * Rebuilds in-memory indexes from database at startup.
 */
#ifndef ASSETTRACE_ASSET_SERVICE_H
#define ASSETTRACE_ASSET_SERVICE_H

#include "asset.h"
#include "database.h"
#include "hashmap.h"
#include "inverted_index.h"
#include "trie.h"
#include "graph.h"
#include "claim.h"
#include "min_heap.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Service result codes */
typedef enum {
    SVC_OK = 0,
    SVC_NOT_FOUND,
    SVC_INVALID_INPUT,
    SVC_INVALID_TRANSITION,
    SVC_DUPLICATE,
    SVC_DB_ERROR,
    SVC_ALLOC_ERROR,
    SVC_ALREADY_RETURNED,
    SVC_ASSET_NOT_CLAIMABLE
} ServiceResult;

/* The main application service */
typedef struct {
    Database *db;
    HashMap *asset_map;         /* asset_id (as string) -> Asset* (not owned, for quick lookup) */
    InvertedIndex *search_index;
    Trie *serial_trie;
    Graph *campus_graph;
    ClaimQueue *claim_queue;
} AssetService;

/**
 * Initialize the service — opens DB, seeds if needed, rebuilds indexes.
 * @param db_path  SQLite database path
 * @return         Initialized AssetService, or NULL on failure
 */
AssetService *service_init(const char *db_path);

/**
 * Shutdown the service — frees all DSA structures and closes DB.
 */
void service_shutdown(AssetService *svc);

/* ===== Asset operations ===== */

/**
 * Register a found asset.
 * Inserts into DB, builds index entries, inserts serial into trie.
 * Sets status to REGISTERED.
 * @param out_id  Receives the new asset ID
 * @return        SVC_OK on success
 */
ServiceResult service_register_asset(AssetService *svc, const Asset *asset, int *out_id);

/**
 * Get an asset by ID.
 */
ServiceResult service_get_asset(AssetService *svc, int asset_id, Asset *out);

/**
 * Get all assets with pagination.
 */
AssetList *service_get_all_assets(AssetService *svc, int limit, int offset);

/**
 * Update an asset. Validates transition if status changed.
 * Updates inverted index and trie.
 */
ServiceResult service_update_asset(AssetService *svc, const Asset *asset);

/**
 * Delete an asset. Removes from all indexes.
 */
ServiceResult service_delete_asset(AssetService *svc, int asset_id);

/* ===== Search ===== */

/**
 * Keyword search using inverted index.
 */
SearchResults *service_search(AssetService *svc, const char *query, SearchMode mode);

/**
 * Serial number prefix search using trie.
 */
TrieResults *service_autocomplete_serial(AssetService *svc, const char *prefix, size_t limit);

/* ===== Location & routing ===== */

LocationList *service_get_locations(AssetService *svc);
ServiceResult service_add_location(AssetService *svc, const Location *loc, int *out_id);
ServiceResult service_add_connection(AssetService *svc, int from_id, int to_id,
                                     double weight, const char *description);
ShortestPathResult *service_shortest_path(AssetService *svc, int source_id, int dest_id);

/* ===== Claims ===== */

ServiceResult service_submit_claim(AssetService *svc, Claim *claim, int *out_id);
ClaimList *service_get_claims(AssetService *svc);
ClaimList *service_get_claims_for_asset(AssetService *svc, int asset_id);
const ClaimQueueEntry *service_peek_next_claim(AssetService *svc);
ServiceResult service_review_claim(AssetService *svc, int claim_id,
                                   bool approve, const char *reviewer, const char *notes);

/* ===== Custody ===== */

CustodyEventList *service_get_custody_history(AssetService *svc, int asset_id);

/* ===== Dashboard ===== */

bool service_get_stats(AssetService *svc, DashboardStats *stats);

#ifdef __cplusplus
}
#endif

#endif /* ASSETTRACE_ASSET_SERVICE_H */
