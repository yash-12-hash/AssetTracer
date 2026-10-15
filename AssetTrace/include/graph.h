/**
 * AssetTrace — Weighted Graph with Dijkstra's Algorithm
 *
 * Adjacency list representation. Supports directed or undirected edges.
 * Uses the custom MinHeap for Dijkstra's shortest path.
 *
 * Complexity:
 *   - Add vertex: O(1) amortized
 *   - Add edge: O(1)
 *   - Dijkstra: O((V + E) log V) with binary heap
 */
#ifndef ASSETTRACE_GRAPH_H
#define ASSETTRACE_GRAPH_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GRAPH_INFINITY 1e18
#define MAX_LOCATION_NAME 128
#define MAX_EDGE_DESC 256

/* Edge in adjacency list */
typedef struct GraphEdge {
    int dest_id;
    double weight;
    char description[MAX_EDGE_DESC];
    struct GraphEdge *next;
} GraphEdge;

/* Vertex (location) */
typedef struct {
    int id;
    char name[MAX_LOCATION_NAME];
    GraphEdge *edges;   /* Head of adjacency list */
    bool active;
} GraphVertex;

/* Shortest path result */
typedef struct {
    int *path;           /* Array of vertex IDs from source to dest */
    size_t path_length;  /* Number of vertices in path */
    double total_weight; /* Total path weight */
    bool found;          /* Whether a path was found */
    char **edge_descriptions; /* Descriptions for each edge in path */
    size_t desc_count;
} ShortestPathResult;

/* The graph */
typedef struct {
    GraphVertex *vertices;
    size_t vertex_count;
    size_t vertex_capacity;
    bool directed;       /* true = directed, false = undirected */
} Graph;

/**
 * Create a new graph.
 * @param directed  true for directed graph, false for undirected
 * @return          New Graph, or NULL on failure
 */
Graph *graph_create(bool directed);

/**
 * Add a vertex (location) to the graph.
 * @param id    Unique vertex ID
 * @param name  Location name
 * @return      true on success
 */
bool graph_add_vertex(Graph *g, int id, const char *name);

/**
 * Add an edge between two vertices.
 * For undirected graphs, adds edges in both directions.
 * @param from_id      Source vertex ID
 * @param to_id        Destination vertex ID
 * @param weight       Edge weight (must be >= 0)
 * @param description  Optional edge description
 * @return             true on success, false on invalid args or negative weight
 */
bool graph_add_edge(Graph *g, int from_id, int to_id, double weight,
                    const char *description);

/**
 * Remove an edge.
 * @return true if edge was found and removed
 */
bool graph_remove_edge(Graph *g, int from_id, int to_id);

/**
 * Get neighbors of a vertex.
 * @param out_edges  Receives pointer to edge list head (do NOT free)
 * @return           true if vertex exists
 */
bool graph_get_neighbors(const Graph *g, int vertex_id, GraphEdge **out_edges);

/**
 * Find shortest path using Dijkstra's algorithm with custom MinHeap.
 * @param source_id  Source vertex ID
 * @param dest_id    Destination vertex ID
 * @return           ShortestPathResult (caller must free with shortest_path_free)
 */
ShortestPathResult *graph_shortest_path(const Graph *g, int source_id, int dest_id);

/**
 * Free a ShortestPathResult.
 */
void shortest_path_free(ShortestPathResult *result);

/**
 * Destroy the graph and free all memory.
 */
void graph_destroy(Graph *g);

/**
 * Get vertex name by ID.
 * @return Pointer to name string (read-only), or NULL if not found
 */
const char *graph_get_vertex_name(const Graph *g, int vertex_id);

/**
 * Get vertex count.
 */
size_t graph_vertex_count(const Graph *g);

/**
 * Find vertex index by ID. Returns -1 if not found.
 */
int graph_find_vertex(const Graph *g, int vertex_id);

#ifdef __cplusplus
}
#endif

#endif /* ASSETTRACE_GRAPH_H */
