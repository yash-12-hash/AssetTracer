/**
 * AssetTrace — Custom MinHeap
 *
 * Binary min-heap using a dynamically resized array.
 * Supports generic elements via void* with a user-provided comparator.
 *
 * Complexity:
 *   - Push: O(log N)
 *   - Pop:  O(log N)
 *   - Peek: O(1)
 *   - Size: O(1)
 *
 * Used for: Dijkstra's algorithm priority queue, claim scheduling.
 */
#ifndef ASSETTRACE_MIN_HEAP_H
#define ASSETTRACE_MIN_HEAP_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Comparator function type.
 * Returns negative if a < b, 0 if equal, positive if a > b.
 * For a min-heap, the element with the smallest comparator value is at the top.
 */
typedef int (*MinHeapComparator)(const void *a, const void *b);

/**
 * Optional destructor for heap elements.
 */
typedef void (*MinHeapElementFree)(void *element);

typedef struct {
    void **data;              /* Array of element pointers */
    size_t size;              /* Current number of elements */
    size_t capacity;          /* Allocated capacity */
    MinHeapComparator cmp;    /* Comparator function */
    MinHeapElementFree efree; /* Optional element destructor */
} MinHeap;

/**
 * Create a new min-heap.
 * @param initial_capacity  Starting capacity (0 uses default 16)
 * @param cmp               Comparator function (MUST NOT be NULL)
 * @param efree             Optional element destructor (may be NULL)
 * @return                  Pointer to new MinHeap, or NULL on failure
 */
MinHeap *minheap_create(size_t initial_capacity, MinHeapComparator cmp,
                        MinHeapElementFree efree);

/**
 * Push an element onto the heap.
 * @return true on success, false on allocation failure or NULL args
 */
bool minheap_push(MinHeap *heap, void *element);

/**
 * Peek at the top (minimum) element without removing it.
 * @return Pointer to the top element, or NULL if heap is empty/NULL
 */
void *minheap_peek(const MinHeap *heap);

/**
 * Pop the top (minimum) element.
 * The caller takes ownership of the returned element.
 * @return Pointer to the popped element, or NULL if heap is empty/NULL
 */
void *minheap_pop(MinHeap *heap);

/**
 * Check if the heap is empty.
 */
bool minheap_is_empty(const MinHeap *heap);

/**
 * Get the number of elements in the heap.
 */
size_t minheap_size(const MinHeap *heap);

/**
 * Destroy the heap. If efree is set, all remaining elements are freed.
 */
void minheap_destroy(MinHeap *heap);

#ifdef __cplusplus
}
#endif

#endif /* ASSETTRACE_MIN_HEAP_H */
