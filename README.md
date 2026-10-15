# AssetTrace — Enterprise Asset Recovery & Secure Custody System

> **High-Performance C11 Data Structures & Algorithms (DSA) Implementation**  
> Complete with SQLite Persistence, Embedded Mongoose HTTP Server, REST APIs, and Web Frontend.

---

## 📌 Overview

**AssetTrace** is an enterprise-grade lost-and-found asset tracking and secure chain-of-custody recovery system engineered in pure **C11**. Built specifically to showcase rigorous Data Structures and Algorithms principles, AssetTrace replaces naive sequential searching and unindexed storage with specialized, in-memory data structures optimized for sub-millisecond retrieval, prefix searching, full-text token indexing, priority-driven claim triage, and shortest-path campus logistics.

The system combines custom in-memory data structures with an **SQLite** persistence layer and an embedded **Mongoose HTTP REST server**, serving both JSON APIs and a modern responsive frontend dashboard.

```
+-------------------------------------------------------------------------+
|                              AssetTrace UI                              |
|                   (HTML5 / Modern CSS / Vanilla JS)                     |
+------------------------------------+------------------------------------+
                                     | HTTP / REST
+------------------------------------v------------------------------------+
|                         Embedded Mongoose Server                        |
|                     (Port 8080 / JSON Serialization)                    |
+------------------------------------+------------------------------------+
                                     |
+------------------------------------v------------------------------------+
|                           AssetService Layer                            |
|             (Orchestrator, Index Sync & Business Validation)            |
+------+---------------+------------------+------------------+------------+
       |               |                  |                  |
+------v------+ +------v-------+  +-------v--------+  +------v-----+ +----v---+
|   HashMap   | |     Trie     |  | Inverted Index |  |  ClaimQueue| | Graph  |
|  (FNV-1a /  | | (Serial No.  |  |  (Full-Text &  |  | (Min-Heap  | |(Dijkstra|
|  Chaining)  | | Autocomplete)|  |  AND/OR Search)|  |  Priority) | |Routing)|
+------+------+ +------+-------+  +-------+--------+  +------+-----+ +----+---+
       |               |                  |                  |            |
+------v---------------v------------------v------------------v------------v---+
|                            SQLite Database                              |
|        (WAL Mode / Prepared Statements / Seed & Recovery Persistence)   |
+-------------------------------------------------------------------------+
```

---

## 🚀 Core Data Structures & Algorithmic Modules

Every core data structure in AssetTrace is custom-implemented in C from first principles:

### 1. Custom Hash Map (`HashMap` — `hashmap.h` / `src/hashmap.c`)
* **Algorithm**: **FNV-1a** 32-bit hash algorithm (`hashmap_fnv1a`).
* **Collision Resolution**: Separate chaining using singly-linked lists of key-value pairs.
* **Dynamic Rehashing**: Automatically doubles bucket capacity whenever the load factor exceeds **0.75**, maintaining average $O(1)$ performance.
* **Role**: Primary index for instant $O(1)$ asset record retrieval by ID, keyword-to-posting-list lookup in the search index, and memory-cached state.

### 2. Prefix Trie (`Trie` — `trie.h` / `src/trie.c`)
* **Alphabet**: 38 branching factors (`A-Z`, `0-9`, `-`, `_`).
* **Normalization**: Case-insensitive normalization with fast alphanumeric lookup indexing.
* **Payload**: Terminal nodes store dynamically expandable arrays of associated asset IDs.
* **Pruning**: Pruning on deletion removes unnecessary nodes when associations are severed.
* **Role**: Sub-millisecond serial number prefix matching, bar-code lookup, and real-time frontend search autocompletion ($O(L)$ where $L$ is string length).

### 3. Inverted Search Index (`InvertedIndex` — `inverted_index.h` / `src/inverted_index.c`)
* **Tokenization**: Lowercase conversion, punctuation stripping, whitespace tokenization, and stop-word filtering.
* **Relevance Weighting**: Dynamic scoring based on matched fields (Title: 3.0x, Category: 2.0x, Description: 1.0x).
* **Query Evaluation**:
  * **AND Mode**: Posting list intersection algorithm ($O(K \times P_{\max})$).
  * **OR Mode**: Posting list union with score aggregation ($O(\sum P_i)$).
* **Role**: Multi-keyword search across titles, descriptions, categories, and tags.

### 4. Min-Heap Priority Queue (`MinHeap` & `ClaimQueue` — `min_heap.h`, `claim.h` / `src/min_heap.c`, `src/claim.c`)
* **Data Structure**: Binary min-heap with dynamic array resizing and manual `sift_up` / `sift_down`.
* **Urgency Scoring Function**:
  $$\text{Urgency} = (w_{\text{crit}} \times \text{Criticality}) + (w_{\text{conf}} \times \text{Confidence}) + (w_{\text{age}} \times \text{Normalized Age})$$
* **Max-Priority Transformation**: Uses negated urgency scores to enable $O(\log N)$ extraction of highest-urgency claims on top of a min-heap.
* **Lazy Invalidation**: Claim cancellation/updates employ version stamps, discarding stale records on pop without requiring expensive arbitrary-element heap deletions.
* **Role**: Prioritizing claim verification queues for compliance officers and security personnel.

### 5. Weighted Graph & Dijkstra Shortest Path (`Graph` — `graph.h` / `src/graph.c`)
* **Representation**: Adjacency list with weighted, directed or undirected edges.
* **Routing Algorithm**: Dijkstra’s shortest path using the custom `MinHeap` as the priority queue ($O((V + E) \log V)$).
* **Route Reconstruction**: Step-by-step custody pathway with edge descriptions and physical distance/transit weights.
* **Role**: Finding the fastest and most secure transfer route between campus discovery points, holding lockers, forensic labs, and return desks.

---

## 📊 Complexity Analysis Matrix

| Module | Operation | Average Complexity | Worst Case | Space Complexity |
| :--- | :--- | :---: | :---: | :---: |
| **HashMap** | `hashmap_get` / `hashmap_put` | $O(1)$ | $O(N)$ (clustering) | $O(N)$ |
| **HashMap** | `hashmap_resize` (rehashing) | $O(N)$ | $O(N)$ | $O(N)$ |
| **Trie** | `trie_insert` / `trie_search_exact` | $O(L)$ | $O(L)$ | $O(L \times |\Sigma|)$ |
| **Trie** | `trie_search_prefix` | $O(L + K)$ | $O(L + N)$ | $O(K)$ |
| **Inverted Index** | `inverted_index_add_asset` | $O(T \times \bar{L})$ | $O(T \times \bar{L})$ | $O(T \times \text{postings})$ |
| **Inverted Index** | `inverted_index_search` (AND) | $O(K \times P_{\min})$ | $O(K \times P_{\max})$ | $O(R \log R)$ |
| **MinHeap** | `minheap_push` / `minheap_pop` | $O(\log N)$ | $O(\log N)$ | $O(N)$ |
| **MinHeap** | `minheap_peek` | $O(1)$ | $O(1)$ | $O(1)$ |
| **Graph** | `graph_add_edge` / `graph_add_vertex` | $O(1)$ | $O(1)$ | $O(V + E)$ |
| **Graph** | `graph_shortest_path` (Dijkstra) | $O((V + E) \log V)$ | $O((V + E) \log V)$ | $O(V)$ |

*Legend: $N$ = total items, $L$ = string length, $K$ = query terms / matched prefixes, $T$ = terms per document, $R$ = result count, $V$ = vertices, $E$ = edges.*

---

## 🗂️ Project Directory Structure

```
AssetTrace/
├── bin/
│   ├── assettrace.exe         # Compiled server executable
│   └── test_runner.exe        # Compiled automated DSA test runner
├── data/
│   └── assettrace.db          # SQLite persistent database (WAL mode)
├── docs/                      # Architectural, API & DSA documentation
│   ├── API.md
│   ├── ARCHITECTURE.md
│   ├── DSA_ANALYSIS.md
│   └── USER_MANUAL.md
├── frontend/                  # Web dashboard UI
│   ├── index.html             # Dashboard view
│   ├── style.css              # Custom styling
│   └── app.js                 # Frontend interactions & API bindings
├── include/                   # Public C header files
│   ├── asset.h                # Core models (Asset, Claim, CustodyEvent, Location)
│   ├── asset_service.h        # Service orchestration interface
│   ├── claim.h                # Priority queue claim scheduling interface
│   ├── database.h             # SQLite abstraction layer
│   ├── graph.h                # Campus graph & Dijkstra routing interface
│   ├── hashmap.h              # Generic FNV-1a separate-chaining hashmap
│   ├── inverted_index.h       # Full-text inverted search index interface
│   ├── min_heap.h             # Generic binary min-heap interface
│   └── trie.h                 # Alphanumeric prefix trie interface
├── lib/                       # Third-party embedded dependencies
│   ├── mongoose.c / .h        # Embedded Mongoose HTTP networking library
│   └── sqlite3.c / .h         # Amalgamated SQLite3 engine
├── scripts/                   # Automated build and run scripts
│   ├── build.bat              # Build automation
│   ├── run.bat                # Start server
│   └── test.bat               # Run test suite
├── src/                       # Module implementations
│   ├── asset.c                # Model validators and state transition machine
│   ├── asset_service.c        # Service layer synchronizing DB and DSAs
│   ├── claim.c                # Claim priority queue implementation
│   ├── database.c             # SQLite CRUD, migrations, and transactions
│   ├── graph.c                # Graph and Dijkstra implementation
│   ├── hashmap.c              # Hashmap implementation
│   ├── inverted_index.c       # Inverted index search implementation
│   ├── main.c                 # Application CLI entrypoint and signal handling
│   ├── min_heap.c             # Binary min-heap implementation
│   ├── server.c               # Mongoose REST endpoint routing
│   └── trie.c                 # Prefix trie implementation
├── tests/                     # Unit and integration test suites
│   ├── test_graph.c
│   ├── test_hashmap.c
│   ├── test_inverted_index.c
│   ├── test_min_heap.c
│   ├── test_runner.c          # Master test suite orchestrator
│   ├── test_service.c
│   └── test_trie.c
├── CMakeLists.txt             # CMake configuration
├── Makefile                   # GNU Make build configuration
└── README.md                  # System overview and guide
```

---

## 🛠️ Build and Execution Guide

### Prerequisites
* **Compiler**: MinGW-w64 / GCC (with C11 support) or Clang.
* **Operating System**: Windows, Linux, or macOS.
* **Libraries**: Standard C runtime (`-lm` for math, `-lpthread` / Windows socket libraries for Mongoose).

### 1. Running the Automated Test Suites
AssetTrace includes comprehensive unit tests verifying data structure contracts, edge cases, collision handling, and service synchronization:

```powershell
# Run the compiled test runner
.\bin\test_runner.exe
```

**Test Suite Coverage:**
* `HashMap`: Key insertions, updates, collisions, rehashing, deletions, memory cleanup (182 assertions).
* `Trie`: Case-insensitivity, prefix search, duplicate serials, node pruning (25 assertions).
* `Inverted Index`: Tokenization, punctuation filtering, AND/OR queries, relevance scoring (18 assertions).
* `MinHeap & Priority Queue`: Heap properties, custom comparisons, urgency order, lazy invalidation (25 assertions).
* `Graph & Dijkstra`: Graph topology, cycle handling, edge relaxation, shortest paths (12 assertions).
* `AssetService Integration`: Database boot, index reconstruction, end-to-end CRUD (19 assertions).

### 2. Starting the AssetTrace Server
Launch the HTTP server and API daemon:

```powershell
.\bin\assettrace.exe --port 8080 --db data/assettrace.db --web frontend
```

#### CLI Options:
| Flag | Description | Default |
| :--- | :--- | :--- |
| `--port <PORT>` | HTTP port to listen on | `8080` |
| `--db <PATH>` | Path to SQLite database file | `data/assettrace.db` |
| `--web <PATH>` | Static web assets directory | `frontend` |
| `--seed` | Force database re-seeding with sample data | *Disabled* |
| `--help` | Display usage instructions | — |

Once running, navigate to `http://localhost:8080` in your web browser to open the UI.

---

## 📡 REST API Reference

The server exposes a clean JSON REST API:

### Assets
* `GET /api/assets?limit=20&offset=0` — Retrieve paginated list of assets.
* `GET /api/assets/:id` — Get detailed asset record with status and metadata.
* `POST /api/assets` — Register a new found asset (populates DB, Trie, and Inverted Index).
* `PUT /api/assets/:id` — Update asset status with state transition validation.
* `DELETE /api/assets/:id` — Delete asset and purge index entries.

### Search & Autocomplete
* `GET /api/search?q=:query&mode=and` — Full-text keyword search (`mode=and` or `mode=or`).
* `GET /api/autocomplete?prefix=:prefix` — Serial number prefix auto-complete via Trie.

### Claims & Priority Triage
* `GET /api/claims` — List all registered claims.
* `GET /api/claims/next` — Peek at the highest-urgency claim from the priority queue.
* `POST /api/claims` — Submit an ownership claim (calculates urgency score and enqueues).
* `POST /api/claims/:id/review` — Approve or reject a claim (updates custody workflow).

### Custody Routing & Network
* `GET /api/locations` — Retrieve all facilities, lockers, and custody points.
* `GET /api/route?from=:source_id&to=:dest_id` — Compute Dijkstra shortest transfer route.
* `GET /api/custody/:asset_id` — Chain-of-custody audit trail for an asset.

### Metrics
* `GET /api/stats` — Total assets, in-custody counts, returned percentage, and active claims.

---

## 🔄 Asset Custody Lifecycle & State Machine

Asset transitions adhere to a strict state machine to prevent fraud and ensure audit compliance:

```
[FOUND] ────────► [REGISTERED] ────────► [IN_CUSTODY]
                         │                      │
                         ▼                      ▼
                 [CLAIM_PENDING] ◄──────────────┘
                         │
                         ▼
             [UNDER_VERIFICATION]
                   │           │
         (Approved)│           │(Rejected)
                   ▼           ▼
          [RETURN_APPROVED]  [IN_CUSTODY]
                   │
                   ▼
              [RETURNED]
                   │
                   ▼
               [CLOSED]
```

Invalid transitions (e.g. attempting to mark a `CLOSED` asset as `FOUND`, or bypassing `UNDER_VERIFICATION` directly to `RETURNED`) are rejected with `SVC_INVALID_TRANSITION`.

---

## 📄 License
This project is developed for educational and academic demonstration within the Foundations of Data Structures (FODS) curriculum. Released under the MIT License.
