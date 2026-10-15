/**
 * AssetTrace — Claim Priority Queue
 *
 * Uses the custom MinHeap to schedule claims by urgency.
 * Higher urgency = extracted first (uses negated score for min-heap).
 * Deterministic tie-breaking: submission time, then claim ID.
 */
#ifndef ASSETTRACE_CLAIM_H
#define ASSETTRACE_CLAIM_H

#include "asset.h"
#include "min_heap.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Claim queue entry for the heap */
typedef struct {
    int claim_id;
    int asset_id;
    double urgency_score;
    time_t submitted_at;
    ClaimStatus status;
    int version;              /* For lazy invalidation */
} ClaimQueueEntry;

/* The claim queue */
typedef struct {
    MinHeap *heap;
    int current_version;     /* Tracks version for lazy invalidation */
    size_t active_count;     /* Count of non-invalidated entries */
} ClaimQueue;

/**
 * Create a new claim queue.
 * @return New ClaimQueue, or NULL on failure
 */
ClaimQueue *claim_queue_create(void);

/**
 * Enqueue a claim for priority scheduling.
 * @return true on success
 */
bool claim_queue_enqueue(ClaimQueue *q, const Claim *claim,
                         AssetCriticality asset_criticality);

/**
 * Peek at the highest-priority claim without removing it.
 * Skips invalidated entries.
 * @return Pointer to entry (do NOT free), or NULL if empty
 */
const ClaimQueueEntry *claim_queue_peek(ClaimQueue *q);

/**
 * Pop the highest-priority claim.
 * Caller takes ownership of the returned entry and must free it.
 * @return Pointer to entry, or NULL if empty
 */
ClaimQueueEntry *claim_queue_pop(ClaimQueue *q);

/**
 * Remove/invalidate entries for a specific claim.
 * Uses lazy invalidation (version increment).
 */
void claim_queue_invalidate(ClaimQueue *q, int claim_id);

/**
 * Destroy the claim queue.
 */
void claim_queue_destroy(ClaimQueue *q);

/**
 * Get number of active (non-invalidated) entries.
 */
size_t claim_queue_size(const ClaimQueue *q);

#ifdef __cplusplus
}
#endif

#endif /* ASSETTRACE_CLAIM_H */
