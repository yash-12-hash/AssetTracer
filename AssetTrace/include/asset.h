/**
 * AssetTrace — Asset and Claim Data Types
 */
#ifndef ASSETTRACE_ASSET_H
#define ASSETTRACE_ASSET_H

#include <stddef.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Asset status lifecycle */
typedef enum {
    STATUS_FOUND = 0,
    STATUS_REGISTERED,
    STATUS_IN_CUSTODY,
    STATUS_CLAIM_PENDING,
    STATUS_UNDER_VERIFICATION,
    STATUS_RETURN_APPROVED,
    STATUS_RETURNED,
    STATUS_CLOSED
} AssetStatus;

/* Asset criticality levels */
typedef enum {
    CRITICALITY_LOW = 1,
    CRITICALITY_MEDIUM = 2,
    CRITICALITY_HIGH = 3,
    CRITICALITY_CRITICAL = 4
} AssetCriticality;

/* Claim status */
typedef enum {
    CLAIM_PENDING = 0,
    CLAIM_UNDER_REVIEW,
    CLAIM_APPROVED,
    CLAIM_REJECTED,
    CLAIM_CANCELLED
} ClaimStatus;

#define MAX_TITLE        256
#define MAX_CATEGORY      64
#define MAX_DESCRIPTION 1024
#define MAX_SERIAL       128
#define MAX_LOCATION     128
#define MAX_EVIDENCE     512
#define MAX_REVIEWER      64
#define MAX_NOTES        512

/* Asset record */
typedef struct {
    int id;
    char title[MAX_TITLE];
    char category[MAX_CATEGORY];
    char description[MAX_DESCRIPTION];
    char serial_number[MAX_SERIAL];
    int found_location_id;
    int custody_location_id;
    AssetCriticality criticality;
    AssetStatus status;
    time_t created_at;
    time_t updated_at;
} Asset;

/* Claim record */
typedef struct {
    int id;
    int asset_id;
    char claimant_name[MAX_TITLE];
    char evidence[MAX_EVIDENCE];
    double confidence;           /* 0.0 to 1.0 */
    ClaimStatus status;
    time_t submitted_at;
    time_t reviewed_at;
    char reviewer[MAX_REVIEWER];
    char notes[MAX_NOTES];
    double urgency_score;        /* Calculated score for priority */
} Claim;

/* Custody event record */
typedef struct {
    int id;
    int asset_id;
    int from_location_id;
    int to_location_id;
    char event_type[MAX_CATEGORY];   /* e.g., "INTAKE", "TRANSFER", "RETURN" */
    char handler[MAX_TITLE];
    char notes[MAX_NOTES];
    time_t timestamp;
} CustodyEvent;

/* Location record */
typedef struct {
    int id;
    char name[MAX_LOCATION];
    char description[MAX_DESCRIPTION];
    bool is_secure;
} Location;

/* Helper functions */
const char *asset_status_to_string(AssetStatus status);
AssetStatus asset_status_from_string(const char *str);
const char *claim_status_to_string(ClaimStatus status);
ClaimStatus claim_status_from_string(const char *str);
const char *criticality_to_string(AssetCriticality crit);
AssetCriticality criticality_from_string(const char *str);

/**
 * Check if a status transition is valid.
 */
bool is_valid_transition(AssetStatus from, AssetStatus to);

/**
 * Calculate claim urgency score.
 * Formula: criticality_weight * norm_crit + confidence_weight * confidence + age_weight * norm_age
 */
double calculate_urgency(AssetCriticality criticality, double confidence,
                        time_t submitted_at, time_t now);

#ifdef __cplusplus
}
#endif

#endif /* ASSETTRACE_ASSET_H */
