/**
 * AssetTrace — Custom Hash Map
 * 
 * String-keyed hash map using separate chaining with linked-list nodes.
 * Hash function: FNV-1a (32-bit).
 * Load factor threshold: 0.75 (triggers resize to 2x capacity).
 * 
 * Ownership: Keys are duplicated (owned by the map). Values are NOT owned
 * by the map — caller is responsible for freeing values before removal
 * or must provide a value_free function at creation time.
 *
 * Complexity:
 *   - Average-case lookup/insert/delete: O(1) under uniform hashing
 *   - Worst-case (all keys collide): O(N)
 *   - Resize: O(N) amortized
 */
#ifndef ASSETTRACE_HASHMAP_H
#define ASSETTRACE_HASHMAP_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Result codes for hashmap operations */
typedef enum {
    HASHMAP_OK = 0,
    HASHMAP_NOT_FOUND,
    HASHMAP_ALLOC_FAILURE,
    HASHMAP_NULL_ARG,
    HASHMAP_DUPLICATE  /* For insert-only mode if needed */
} HashMapResult;

/* A single entry in the chain (linked list node) */
typedef struct HashMapEntry {
    char *key;                    /* Owned copy of the key string */
    void *value;                  /* NOT owned — caller manages lifetime */
    struct HashMapEntry *next;    /* Next entry in the chain */
} HashMapEntry;

/* Function pointer type for freeing values */
typedef void (*HashMapValueFree)(void *value);

/* The hash map structure */
typedef struct {
    HashMapEntry **buckets;       /* Array of bucket heads */
    size_t capacity;              /* Number of buckets */
    size_t size;                  /* Number of stored entries */
    double load_factor_threshold; /* Resize when size/capacity > this */
    HashMapValueFree value_free;  /* Optional value destructor */
} HashMap;

/**
 * Create a new hash map.
 * @param initial_capacity  Starting number of buckets (0 uses default 16)
 * @param value_free        Optional function to free values on remove/destroy (may be NULL)
 * @return                  Pointer to new HashMap, or NULL on allocation failure
 */
HashMap *hashmap_create(size_t initial_capacity, HashMapValueFree value_free);

/**
 * Insert or update a key-value pair.
 * If key exists, the old value is freed (if value_free is set) and replaced.
 * The key string is duplicated internally.
 * @return HASHMAP_OK on success, HASHMAP_ALLOC_FAILURE on memory error,
 *         HASHMAP_NULL_ARG if map or key is NULL
 */
HashMapResult hashmap_put(HashMap *map, const char *key, void *value);

/**
 * Retrieve a value by key.
 * @param out_value  If non-NULL and key is found, *out_value is set to the stored value
 * @return           HASHMAP_OK if found, HASHMAP_NOT_FOUND if missing,
 *                   HASHMAP_NULL_ARG if map or key is NULL
 */
HashMapResult hashmap_get(const HashMap *map, const char *key, void **out_value);

/**
 * Check if a key exists.
 * @return true if key is present, false otherwise (including NULL args)
 */
bool hashmap_contains(const HashMap *map, const char *key);

/**
 * Remove a key-value pair. Frees the key. If value_free is set, frees the value.
 * @return HASHMAP_OK if removed, HASHMAP_NOT_FOUND if missing,
 *         HASHMAP_NULL_ARG if map or key is NULL
 */
HashMapResult hashmap_remove(HashMap *map, const char *key);

/**
 * Manually trigger a resize to new_capacity buckets.
 * All entries are rehashed. If allocation fails, the map is unchanged.
 * @return HASHMAP_OK on success, HASHMAP_ALLOC_FAILURE on failure
 */
HashMapResult hashmap_resize(HashMap *map, size_t new_capacity);

/**
 * Return the number of entries in the map.
 */
size_t hashmap_size(const HashMap *map);

/**
 * Destroy the hash map and free all keys.
 * If value_free is set, all values are freed too.
 * After calling this, the map pointer is invalid.
 */
void hashmap_destroy(HashMap *map);

/* ----- Iteration support ----- */

/**
 * Iterator for traversing all entries.
 * Usage:
 *   HashMapIterator it;
 *   hashmap_iterator_init(map, &it);
 *   while (hashmap_iterator_next(&it)) {
 *       printf("%s -> %p\n", it.key, it.value);
 *   }
 */
typedef struct {
    const HashMap *map;
    size_t bucket_index;
    HashMapEntry *current;
    const char *key;              /* Current key (read-only) */
    void *value;                  /* Current value */
} HashMapIterator;

void hashmap_iterator_init(const HashMap *map, HashMapIterator *it);
bool hashmap_iterator_next(HashMapIterator *it);

/* ----- FNV-1a hash (exposed for testing) ----- */
unsigned int hashmap_fnv1a(const char *key);

#ifdef __cplusplus
}
#endif

#endif /* ASSETTRACE_HASHMAP_H */
