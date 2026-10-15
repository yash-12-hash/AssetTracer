/**
 * AssetTrace — Graph Implementation with Dijkstra's Algorithm
 *
 * Uses custom MinHeap for the priority queue in Dijkstra's.
 * Undirected graph by default (adds edges in both directions).
 */
#include "graph.h"
#include "min_heap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

#define DEFAULT_VERTEX_CAPACITY 16

/* ===== Dijkstra heap entry ===== */
typedef struct {
    int vertex_id;
    double distance;
} DijkstraEntry;

static int dijkstra_cmp(const void *a, const void *b) {
    const DijkstraEntry *da = (const DijkstraEntry *)a;
    const DijkstraEntry *db = (const DijkstraEntry *)b;
    if (da->distance < db->distance) return -1;
    if (da->distance > db->distance) return 1;
    /* Tie-break by vertex ID for determinism */
    if (da->vertex_id < db->vertex_id) return -1;
    if (da->vertex_id > db->vertex_id) return 1;
    return 0;
}

/* ===== Internal helpers ===== */

int graph_find_vertex(const Graph *g, int vertex_id) {
    if (!g) return -1;
    for (size_t i = 0; i < g->vertex_count; i++) {
        if (g->vertices[i].active && g->vertices[i].id == vertex_id) {
            return (int)i;
        }
    }
    return -1;
}

static GraphEdge *edge_create(int dest_id, double weight, const char *description) {
    GraphEdge *e = (GraphEdge *)malloc(sizeof(GraphEdge));
    if (!e) return NULL;
    e->dest_id = dest_id;
    e->weight = weight;
    if (description) {
        strncpy(e->description, description, MAX_EDGE_DESC - 1);
        e->description[MAX_EDGE_DESC - 1] = '\0';
    } else {
        e->description[0] = '\0';
    }
    e->next = NULL;
    return e;
}

/* ===== Public API ===== */

Graph *graph_create(bool directed) {
    Graph *g = (Graph *)malloc(sizeof(Graph));
    if (!g) return NULL;

    g->vertices = (GraphVertex *)calloc(DEFAULT_VERTEX_CAPACITY, sizeof(GraphVertex));
    if (!g->vertices) { free(g); return NULL; }

    g->vertex_count = 0;
    g->vertex_capacity = DEFAULT_VERTEX_CAPACITY;
    g->directed = directed;
    return g;
}

bool graph_add_vertex(Graph *g, int id, const char *name) {
    if (!g || !name) return false;

    /* Check for duplicate */
    if (graph_find_vertex(g, id) >= 0) return false;

    /* Grow if needed */
    if (g->vertex_count >= g->vertex_capacity) {
        size_t new_cap = g->vertex_capacity * 2;
        GraphVertex *new_v = (GraphVertex *)realloc(g->vertices, new_cap * sizeof(GraphVertex));
        if (!new_v) return false;
        g->vertices = new_v;
        g->vertex_capacity = new_cap;
    }

    GraphVertex *v = &g->vertices[g->vertex_count];
    v->id = id;
    strncpy(v->name, name, MAX_LOCATION_NAME - 1);
    v->name[MAX_LOCATION_NAME - 1] = '\0';
    v->edges = NULL;
    v->active = true;

    g->vertex_count++;
    return true;
}

bool graph_add_edge(Graph *g, int from_id, int to_id, double weight,
                    const char *description) {
    if (!g) return false;
    if (weight < 0) return false;

    int from_idx = graph_find_vertex(g, from_id);
    int to_idx = graph_find_vertex(g, to_id);
    if (from_idx < 0 || to_idx < 0) return false;

    /* Add forward edge */
    GraphEdge *e = edge_create(to_id, weight, description);
    if (!e) return false;
    e->next = g->vertices[from_idx].edges;
    g->vertices[from_idx].edges = e;

    /* For undirected, add reverse edge */
    if (!g->directed) {
        GraphEdge *rev = edge_create(from_id, weight, description);
        if (!rev) return false;  /* Forward edge already added — minor inconsistency */
        rev->next = g->vertices[to_idx].edges;
        g->vertices[to_idx].edges = rev;
    }

    return true;
}

bool graph_remove_edge(Graph *g, int from_id, int to_id) {
    if (!g) return false;

    int from_idx = graph_find_vertex(g, from_id);
    if (from_idx < 0) return false;

    GraphEdge *prev = NULL;
    GraphEdge *e = g->vertices[from_idx].edges;
    bool removed = false;

    while (e) {
        if (e->dest_id == to_id) {
            if (prev) prev->next = e->next;
            else g->vertices[from_idx].edges = e->next;
            free(e);
            removed = true;
            break;
        }
        prev = e;
        e = e->next;
    }

    /* For undirected, remove reverse edge too */
    if (!g->directed && removed) {
        int to_idx = graph_find_vertex(g, to_id);
        if (to_idx >= 0) {
            prev = NULL;
            e = g->vertices[to_idx].edges;
            while (e) {
                if (e->dest_id == from_id) {
                    if (prev) prev->next = e->next;
                    else g->vertices[to_idx].edges = e->next;
                    free(e);
                    break;
                }
                prev = e;
                e = e->next;
            }
        }
    }

    return removed;
}

bool graph_get_neighbors(const Graph *g, int vertex_id, GraphEdge **out_edges) {
    if (!g || !out_edges) return false;

    int idx = graph_find_vertex(g, vertex_id);
    if (idx < 0) return false;

    *out_edges = g->vertices[idx].edges;
    return true;
}

ShortestPathResult *graph_shortest_path(const Graph *g, int source_id, int dest_id) {
    ShortestPathResult *result = (ShortestPathResult *)malloc(sizeof(ShortestPathResult));
    if (!result) return NULL;
    result->path = NULL;
    result->path_length = 0;
    result->total_weight = GRAPH_INFINITY;
    result->found = false;
    result->edge_descriptions = NULL;
    result->desc_count = 0;

    if (!g || g->vertex_count == 0) return result;

    int src_idx = graph_find_vertex(g, source_id);
    int dst_idx = graph_find_vertex(g, dest_id);
    if (src_idx < 0 || dst_idx < 0) return result;

    /* Source == destination: zero-length route */
    if (source_id == dest_id) {
        result->found = true;
        result->total_weight = 0.0;
        result->path = (int *)malloc(sizeof(int));
        if (result->path) {
            result->path[0] = source_id;
            result->path_length = 1;
        }
        return result;
    }

    size_t n = g->vertex_count;

    /* Allocate Dijkstra arrays */
    double *dist = (double *)malloc(n * sizeof(double));
    int *pred = (int *)malloc(n * sizeof(int));
    bool *visited = (bool *)calloc(n, sizeof(bool));
    char **edge_descs = (char **)calloc(n, sizeof(char *));

    if (!dist || !pred || !visited || !edge_descs) {
        free(dist); free(pred); free(visited); free(edge_descs);
        return result;
    }

    for (size_t i = 0; i < n; i++) {
        dist[i] = GRAPH_INFINITY;
        pred[i] = -1;
    }
    dist[src_idx] = 0.0;

    /* Create MinHeap for Dijkstra */
    MinHeap *heap = minheap_create(n, dijkstra_cmp, free);
    if (!heap) {
        free(dist); free(pred); free(visited); free(edge_descs);
        return result;
    }

    DijkstraEntry *start = (DijkstraEntry *)malloc(sizeof(DijkstraEntry));
    if (!start) {
        minheap_destroy(heap);
        free(dist); free(pred); free(visited); free(edge_descs);
        return result;
    }
    start->vertex_id = source_id;
    start->distance = 0.0;
    minheap_push(heap, start);

    while (!minheap_is_empty(heap)) {
        DijkstraEntry *entry = (DijkstraEntry *)minheap_pop(heap);
        int uid = entry->vertex_id;
        double udist = entry->distance;
        free(entry);

        int uidx = graph_find_vertex(g, uid);
        if (uidx < 0) continue;

        /* Skip stale entries */
        if (visited[uidx]) continue;
        if (udist > dist[uidx]) continue;

        visited[uidx] = true;

        /* Early exit if we reached destination */
        if (uid == dest_id) break;

        /* Relax neighbors */
        GraphEdge *e = g->vertices[uidx].edges;
        while (e) {
            int vidx = graph_find_vertex(g, e->dest_id);
            if (vidx >= 0 && !visited[vidx]) {
                double alt = dist[uidx] + e->weight;
                if (alt < dist[vidx]) {
                    dist[vidx] = alt;
                    pred[vidx] = uidx;

                    /* Store edge description */
                    free(edge_descs[vidx]);
                    edge_descs[vidx] = NULL;
                    if (e->description[0] != '\0') {
                        size_t dlen = strlen(e->description);
                        edge_descs[vidx] = (char *)malloc(dlen + 1);
                        if (edge_descs[vidx]) {
                            memcpy(edge_descs[vidx], e->description, dlen + 1);
                        }
                    }

                    DijkstraEntry *ne = (DijkstraEntry *)malloc(sizeof(DijkstraEntry));
                    if (ne) {
                        ne->vertex_id = e->dest_id;
                        ne->distance = alt;
                        minheap_push(heap, ne);
                    }
                }
            }
            e = e->next;
        }
    }

    minheap_destroy(heap);

    /* Check if destination was reached */
    if (dist[dst_idx] < GRAPH_INFINITY) {
        result->found = true;
        result->total_weight = dist[dst_idx];

        /* Reconstruct path */
        int *rev_path = (int *)malloc(n * sizeof(int));
        if (rev_path) {
            size_t plen = 0;
            int current = dst_idx;
            while (current >= 0) {
                rev_path[plen++] = current;
                if (current == src_idx) break;
                current = pred[current];
            }

            result->path = (int *)malloc(plen * sizeof(int));
            result->edge_descriptions = (char **)calloc(plen, sizeof(char *));
            if (result->path) {
                result->path_length = plen;
                for (size_t i = 0; i < plen; i++) {
                    int idx = rev_path[plen - 1 - i];
                    result->path[i] = g->vertices[idx].id;
                }

                /* Copy edge descriptions */
                if (result->edge_descriptions) {
                    result->desc_count = 0;
                    for (size_t i = 0; i < plen; i++) {
                        int idx = rev_path[plen - 1 - i];
                        if (edge_descs[idx]) {
                            size_t dlen = strlen(edge_descs[idx]);
                            result->edge_descriptions[i] = (char *)malloc(dlen + 1);
                            if (result->edge_descriptions[i]) {
                                memcpy(result->edge_descriptions[i], edge_descs[idx], dlen + 1);
                                result->desc_count++;
                            }
                        }
                    }
                }
            }
            free(rev_path);
        }
    }

    /* Cleanup */
    for (size_t i = 0; i < n; i++) {
        free(edge_descs[i]);
    }
    free(edge_descs);
    free(dist);
    free(pred);
    free(visited);

    return result;
}

void shortest_path_free(ShortestPathResult *result) {
    if (!result) return;
    free(result->path);
    if (result->edge_descriptions) {
        for (size_t i = 0; i < result->path_length; i++) {
            free(result->edge_descriptions[i]);
        }
        free(result->edge_descriptions);
    }
    free(result);
}

const char *graph_get_vertex_name(const Graph *g, int vertex_id) {
    if (!g) return NULL;
    int idx = graph_find_vertex(g, vertex_id);
    if (idx < 0) return NULL;
    return g->vertices[idx].name;
}

size_t graph_vertex_count(const Graph *g) {
    if (!g) return 0;
    return g->vertex_count;
}

void graph_destroy(Graph *g) {
    if (!g) return;

    for (size_t i = 0; i < g->vertex_count; i++) {
        GraphEdge *e = g->vertices[i].edges;
        while (e) {
            GraphEdge *next = e->next;
            free(e);
            e = next;
        }
    }
    free(g->vertices);
    free(g);
}
