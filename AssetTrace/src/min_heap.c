/**
 * AssetTrace — Custom MinHeap Implementation
 *
 * Binary min-heap with dynamic array, manual sift-up/sift-down.
 */
#include "min_heap.h"
#include <stdlib.h>
#include <string.h>

#define DEFAULT_HEAP_CAPACITY 16

/* ===== Internal helpers ===== */

static void swap(void **a, void **b) {
    void *tmp = *a;
    *a = *b;
    *b = tmp;
}

static void sift_up(MinHeap *heap, size_t idx) {
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (heap->cmp(heap->data[idx], heap->data[parent]) < 0) {
            swap(&heap->data[idx], &heap->data[parent]);
            idx = parent;
        } else {
            break;
        }
    }
}

static void sift_down(MinHeap *heap, size_t idx) {
    while (1) {
        size_t smallest = idx;
        size_t left = 2 * idx + 1;
        size_t right = 2 * idx + 2;

        if (left < heap->size && heap->cmp(heap->data[left], heap->data[smallest]) < 0) {
            smallest = left;
        }
        if (right < heap->size && heap->cmp(heap->data[right], heap->data[smallest]) < 0) {
            smallest = right;
        }
        if (smallest != idx) {
            swap(&heap->data[idx], &heap->data[smallest]);
            idx = smallest;
        } else {
            break;
        }
    }
}

static bool ensure_capacity(MinHeap *heap) {
    if (heap->size < heap->capacity) return true;

    size_t new_cap = heap->capacity * 2;
    if (new_cap < heap->capacity) return false; /* overflow check */

    void **new_data = (void **)realloc(heap->data, new_cap * sizeof(void *));
    if (!new_data) return false;

    heap->data = new_data;
    heap->capacity = new_cap;
    return true;
}

/* ===== Public API ===== */

MinHeap *minheap_create(size_t initial_capacity, MinHeapComparator cmp,
                        MinHeapElementFree efree) {
    if (!cmp) return NULL;
    if (initial_capacity == 0) initial_capacity = DEFAULT_HEAP_CAPACITY;

    MinHeap *heap = (MinHeap *)malloc(sizeof(MinHeap));
    if (!heap) return NULL;

    heap->data = (void **)malloc(initial_capacity * sizeof(void *));
    if (!heap->data) {
        free(heap);
        return NULL;
    }

    heap->size = 0;
    heap->capacity = initial_capacity;
    heap->cmp = cmp;
    heap->efree = efree;
    return heap;
}

bool minheap_push(MinHeap *heap, void *element) {
    if (!heap) return false;

    if (!ensure_capacity(heap)) return false;

    heap->data[heap->size] = element;
    sift_up(heap, heap->size);
    heap->size++;
    return true;
}

void *minheap_peek(const MinHeap *heap) {
    if (!heap || heap->size == 0) return NULL;
    return heap->data[0];
}

void *minheap_pop(MinHeap *heap) {
    if (!heap || heap->size == 0) return NULL;

    void *top = heap->data[0];
    heap->size--;

    if (heap->size > 0) {
        heap->data[0] = heap->data[heap->size];
        sift_down(heap, 0);
    }

    return top;
}

bool minheap_is_empty(const MinHeap *heap) {
    if (!heap) return true;
    return heap->size == 0;
}

size_t minheap_size(const MinHeap *heap) {
    if (!heap) return 0;
    return heap->size;
}

void minheap_destroy(MinHeap *heap) {
    if (!heap) return;

    if (heap->efree) {
        for (size_t i = 0; i < heap->size; i++) {
            if (heap->data[i]) {
                heap->efree(heap->data[i]);
            }
        }
    }

    free(heap->data);
    free(heap);
}
