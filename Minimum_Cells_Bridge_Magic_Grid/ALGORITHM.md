# Algorithm — Weighted Shortest Path with 0/1 Costs (three routes to the same optimum)

## First-principles derivation (BFS over the idea space, then depth)

1. **What is being minimized?** The number of `0`-cells turned into `1`s *along some
   top-to-bottom route*. Cells off the route are irrelevant. So the cost of a route =
   number of empty cells on it.
2. **That is a path cost.** Define the cost of *entering* cell `(r,c)`:
   `w = 1` if `runes[r][c] == 0`, `w = 0` if `1`, impassable if `2`.
   Minimizing runes = single-source shortest path with weights in `{0, 1}`.
3. **"Any top cell to any bottom cell"** = multi-source / multi-sink. Standard trick: a
   virtual source `S` above row 0 with an edge into each non-forbidden `(0,c)` of weight
   `w(0,c)`. Answer = `min over c of dist(rows-1, c)`.
4. **Which shortest-path algorithm?** Plain BFS — wrong (weights differ). Dijkstra —
   correct (non-negative weights) but pays a heap log. Weights restricted to `{0,1}` —
   the specialization **0-1 BFS** removes the log. That chain (BFS → Dijkstra → 0-1 BFS)
   is exactly the reasoning an interviewer wants to hear.

---

## Approach ★ — 0-1 BFS (`solution.cpp`)

**Invariant**: the deque always holds nodes of at most two consecutive distance values,
front-to-back nondecreasing.

- Relax a **0-edge** → neighbour has the *same* distance as the popped node → `push_front`.
- Relax a **1-edge** → neighbour belongs to the *next* distance layer → `push_back`.

Both insertions preserve the invariant, so pops occur in nondecreasing distance order —
Dijkstra's correctness argument verbatim — but every operation is O(1).

The deque may contain stale duplicates (a cell improved after being enqueued), handled the
same way as Dijkstra's lazy deletion: a `done` grid, skip on second pop. First pop of any
bottom-row cell is the answer.

**Complexity**: each cell finalized once, each edge relaxed O(1) times → **O(RC) time**,
O(RC) space. This is optimal — you must at least read the grid.

## Approach 1 — Dijkstra (`solution_dijkstra.cpp`)

Same graph, textbook min-heap Dijkstra with lazy deletion. Early-return when a bottom-row
cell is popped. **O(RC log RC)**. Keep this in your pocket as the general tool: it survives
arbitrary non-negative weights (e.g. a follow-up "charging cell (r,c) costs `cost[r][c]`
runes" kills 0-1 BFS but not Dijkstra).

## Approach 2 — Binary search on answer + layered DFS (`solution_binary_search.cpp`)

**Monotone predicate**: `check(k)` = "bottom row reachable charging ≤ k cells". If `k`
works, `k+1` works → binary search the smallest feasible `k`.

`check(k)` — the onion peel:

```
flood FREE through charged cells from row 0 (DFS)          } cost-0 region
boundary := empty cells adjacent to the flooded region
repeat ≤ k times:
    buy the entire boundary (1 rune of path-depth)          } distance layer +1
    flood free through newly reachable charged cells
    boundary := new empty frontier
reached bottom row at any point? → feasible
```

Buying the whole layer at once is safe because any path leaving the flooded region must
first step through exactly one boundary cell — so `check(k)` visits precisely the cells at
weighted distance ≤ k. Each cell is touched once per check → O(RC) per check,
**O(RC log RC)** total. Constant factor is the worst of the three (the grid is re-scanned
per probe) — included because *binary-search-the-answer* is a broadly reusable pattern
whenever feasibility is monotone and cheap.

Note the amusing degenerate fact: running the onion peel **without** the binary search,
just counting layers until the bottom row is hit, is itself an O(RC) algorithm — it is
Dial's bucket variant of Dijkstra, and morally the same algorithm as 0-1 BFS.

---

## Why not…

- **Plain BFS on cells**: treats a charged step and a bought step as equal → overcounts.
- **DP row by row** (`dp[r][c]` from row above): paths may wind upward/sideways
  (see the winding test case) — row-monotone DP misses them.
- **DSU over charged components + BFS on components**: workable (contract charged
  components, 0-1 BFS over the contracted graph) but strictly more code for the same O(RC).

## Benchmarks (1000×1000, 60% empty / 30% charged / 10% forbidden, -O2)

| Implementation | Time |
|---|---|
| 0-1 BFS | 38 ms |
| Dijkstra | 117 ms |
| Binary search + layered DFS | 918 ms |
