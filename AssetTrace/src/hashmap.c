/**
 * AssetTrace — Custom Hash Map Implementation
 * 
 * FNV-1a string hashing with separate chaining.
 * Auto-resizes at load factor > 0.75.
 */
#include "hashmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DEFAULT_CAPACITY     16
#define DEFAULT_LOAD_FACTOR  0.75

/* ===== FNV-1a 32-bit hash ===== */

unsigned int hashmap_fnv1a(const char *key) {
    unsigned int hash = 2166136261u;   /* FNV offset basis */
    while (*key) {
        hash ^= (unsigned char)(*key);
        hash *= 16777619u;             /* FNV prime */
        key++;
    }
    return hash;
}

/* ===== Internal helpers ===== */

static size_t bucket_index(const char *key, size_t capacity) {
    return (size_t)(hashmap_fnv1a(key) % (unsigned int)capacity);
}

static char *strdup_safe(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *dup = (char *)malloc(len + 1);
    if (!dup) return NULL;
    memcpy(dup, s, len + 1);
    return dup;
}

static HashMapEntry *entry_create(const char *key, void *value) {
    HashMapEntry *e = (HashMapEntry *)malloc(sizeof(HashMapEntry));
    if (!e) return NULL;
    e->key = strdup_safe(key);
    if (!e->key) {
        free(e);
        return NULL;
    }
    e->value = value;
    e->next = NULL;
    return e;
}

static void entry_free(HashMapEntry *e, HashMapValueFree value_free) {
    if (!e) return;
    free(e->key);
    if (value_free && e->value) {
        value_free(e->value);
    }
    free(e);
}

/* ===== Public API ===== */

HashMap *hashmap_create(size_t initial_capacity, HashMapValueFree value_free) {
    if (initial_capacity == 0) initial_capacity = DEFAULT_CAPACITY;

    HashMap *map = (HashMap *)malloc(sizeof(HashMap));
    if (!map) return NULL;

    map->buckets = (HashMapEntry **)calloc(initial_capacity, sizeof(HashMapEntry *));
    if (!map->buckets) {
        free(map);
        return NULL;
    }

    map->capacity = initial_capacity;
    map->size = 0;
    map->load_factor_threshold = DEFAULT_LOAD_FACTOR;
    map->value_free = value_free;
    return map;
}

HashMapResult hashmap_resize(HashMap *map, size_t new_capacity) {
    if (!map) return HASHMAP_NULL_ARG;
    if (new_capacity == 0) new_capacity = DEFAULT_CAPACITY;

    HashMapEntry **new_buckets = (HashMapEntry **)calloc(new_capacity, sizeof(HashMapEntry *));
    if (!new_buckets) return HASHMAP_ALLOC_FAILURE;

    /* Rehash all existing entries */
    for (size_t i = 0; i < map->capacity; i++) {
        HashMapEntry *e = map->buckets[i];
        while (e) {
            HashMapEntry *next = e->next;
            size_t idx = bucket_index(e->key, new_capacity);
            e->next = new_buckets[idx];
            new_buckets[idx] = e;
            e = next;
        }
    }

    free(map->buckets);
    map->buckets = new_buckets;
    map->capacity = new_capacity;
    return HASHMAP_OK;
}

HashMapResult hashmap_put(HashMap *map, const char *key, void *value) {
    if (!map || !key) return HASHMAP_NULL_ARG;

    /* Check if key exists — update in place */
    size_t idx = bucket_index(key, map->capacity);
    HashMapEntry *e = map->buckets[idx];
    while (e) {
        if (strcmp(e->key, key) == 0) {
            /* Key exists — replace value */
            if (map->value_free && e->value) {
                map->value_free(e->value);
            }
            e->value = value;
            return HASHMAP_OK;
        }
        e = e->next;
    }

    /* Check load factor before inserting */
    if ((double)(map->size + 1) / (double)map->capacity > map->load_factor_threshold) {
        /* Try to resize — if it fails, we still insert (just degraded performance) */
        hashmap_resize(map, map->capacity * 2);
        /* Recompute index after potential resize */
        idx = bucket_index(key, map->capacity);
    }

    /* Insert new entry at head of chain */
    HashMapEntry *new_entry = entry_create(key, value);
    if (!new_entry) return HASHMAP_ALLOC_FAILURE;

    new_entry->next = map->buckets[idx];
    map->buckets[idx] = new_entry;
    map->size++;

    return HASHMAP_OK;
}

HashMapResult hashmap_get(const HashMap *map, const char *key, void **out_value) {
    if (!map || !key) return HASHMAP_NULL_ARG;

    size_t idx = bucket_index(key, map->capacity);
    HashMapEntry *e = map->buckets[idx];
    while (e) {
        if (strcmp(e->key, key) == 0) {
            if (out_value) *out_value = e->value;
            return HASHMAP_OK;
        }
        e = e->next;
    }
    return HASHMAP_NOT_FOUND;
}

bool hashmap_contains(const HashMap *map, const char *key) {
    if (!map || !key) return false;
    return hashmap_get(map, key, NULL) == HASHMAP_OK;
}

HashMapResult hashmap_remove(HashMap *map, const char *key) {
    if (!map || !key) return HASHMAP_NULL_ARG;

    size_t idx = bucket_index(key, map->capacity);
    HashMapEntry *e = map->buckets[idx];
    HashMapEntry *prev = NULL;

    while (e) {
        if (strcmp(e->key, key) == 0) {
            if (prev) {
                prev->next = e->next;
            } else {
                map->buckets[idx] = e->next;
            }
            entry_free(e, map->value_free);
            map->size--;
            return HASHMAP_OK;
        }
        prev = e;
        e = e->next;
    }
    return HASHMAP_NOT_FOUND;
}

size_t hashmap_size(const HashMap *map) {
    if (!map) return 0;
    return map->size;
}

void hashmap_destroy(HashMap *map) {
    if (!map) return;

    for (size_t i = 0; i < map->capacity; i++) {
        HashMapEntry *e = map->buckets[i];
        while (e) {
            HashMapEntry *next = e->next;
            entry_free(e, map->value_free);
            e = next;
        }
    }
    free(map->buckets);
    free(map);
}

/* ===== Iterator ===== */

void hashmap_iterator_init(const HashMap *map, HashMapIterator *it) {
    if (!it) return;
    it->map = map;
    it->bucket_index = 0;
    it->current = NULL;
    it->key = NULL;
    it->value = NULL;
}

bool hashmap_iterator_next(HashMapIterator *it) {
    if (!it || !it->map) return false;

    /* If we have a current entry, advance to next in chain */
    if (it->current) {
        it->current = it->current->next;
    }

    /* If current is NULL, find next non-empty bucket */
    while (!it->current && it->bucket_index < it->map->capacity) {
        it->current = it->map->buckets[it->bucket_index];
        it->bucket_index++;
    }

    if (it->current) {
        it->key = it->current->key;
        it->value = it->current->value;
        return true;
    }
    return false;
}
