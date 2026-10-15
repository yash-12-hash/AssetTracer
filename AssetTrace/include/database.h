/**
 * AssetTrace — SQLite Database Layer
 *
 * Separates persistence from DSA modules.
 * Uses prepared statements for all operations.
 * Supports transactions for atomic multi-step operations.
 */
#ifndef ASSETTRACE_DATABASE_H
#define ASSETTRACE_DATABASE_H

#include "asset.h"
#include <stdbool.h>
#include <stddef.h>

/* Forward declare sqlite3 to avoid including sqlite3.h in header */
typedef struct sqlite3 sqlite3;

#ifdef __cplusplus
extern "C" {
#endif

/* Database handle */
typedef struct {
    sqlite3 *db;
    bool initialized;
} Database;

/* Result list types */
typedef struct {
    Asset *items;
    size_t count;
    size_t capacity;
} AssetList;

typedef struct {
    Claim *items;
    size_t count;
    size_t capacity;
} ClaimList;

typedef struct {
    CustodyEvent *items;
    size_t count;
    size_t capacity;
} CustodyEventList;

typedef struct {
    Location *items;
    size_t count;
    size_t capacity;
} LocationList;

/* Dashboard stats */
typedef struct {
    int total_assets;
    int found_count;
    int in_custody_count;
    int returned_count;
    int pending_claims;
    int total_claims;
    int total_locations;
} DashboardStats;

/**
 * Open and initialize the database.
 * Creates tables if they don't exist.
 * @param db_path  Path to SQLite database file
 * @return         Database handle, or NULL on failure
 */
Database *db_open(const char *db_path);

/**
 * Close the database.
 */
void db_close(Database *db);

/* ===== Transaction support ===== */
bool db_begin_transaction(Database *db);
bool db_commit(Database *db);
bool db_rollback(Database *db);

/* ===== Asset CRUD ===== */
int db_insert_asset(Database *db, const Asset *asset);  /* Returns new ID, or -1 */
bool db_update_asset(Database *db, const Asset *asset);
bool db_delete_asset(Database *db, int asset_id);
bool db_get_asset(Database *db, int asset_id, Asset *out);
AssetList *db_get_all_assets(Database *db, int limit, int offset);
void asset_list_free(AssetList *list);

/* ===== Location CRUD ===== */
int db_insert_location(Database *db, const Location *loc);
bool db_get_location(Database *db, int loc_id, Location *out);
LocationList *db_get_all_locations(Database *db);
void location_list_free(LocationList *list);

/* ===== Claim CRUD ===== */
int db_insert_claim(Database *db, const Claim *claim);
bool db_update_claim(Database *db, const Claim *claim);
bool db_get_claim(Database *db, int claim_id, Claim *out);
ClaimList *db_get_all_claims(Database *db);
ClaimList *db_get_claims_for_asset(Database *db, int asset_id);
void claim_list_free(ClaimList *list);

/* ===== Custody events ===== */
int db_insert_custody_event(Database *db, const CustodyEvent *event);
CustodyEventList *db_get_custody_history(Database *db, int asset_id);
void custody_event_list_free(CustodyEventList *list);

/* ===== Graph data ===== */
typedef struct {
    int from_id;
    int to_id;
    double weight;
    char description[256];
} ConnectionRecord;

typedef struct {
    ConnectionRecord *items;
    size_t count;
    size_t capacity;
} ConnectionList;

int db_insert_connection(Database *db, int from_id, int to_id, double weight,
                         const char *description);
ConnectionList *db_get_all_connections(Database *db);
void connection_list_free(ConnectionList *list);

/* ===== Dashboard ===== */
bool db_get_dashboard_stats(Database *db, DashboardStats *stats);

/* ===== Seed ===== */
bool db_seed(Database *db);

/* ===== Reset ===== */
bool db_reset(Database *db);

#ifdef __cplusplus
}
#endif

#endif /* ASSETTRACE_DATABASE_H */
