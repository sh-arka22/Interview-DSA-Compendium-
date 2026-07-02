# Algorithm — Product-Graph BFS (grid × conversion automaton)

## Why plain BFS is not enough

Ordinary grid BFS assumes a cell's walkability is fixed. Here it isn't: a blocked cell *may*
be walkable, but only if we spend our one conversion, and that decision is **global** (once
spent, it's gone). So a bare `visited[r][c]` is wrong — arriving at a cell *with* the
conversion still in hand is a genuinely different situation than arriving *without* it. That
extra bit of history is a new state dimension.

## The state

```
state = (row, col, k)      k ∈ {0, 1}   // 0 = conversion available, 1 = spent
```

Encode it flat: `id(r,c,k) = (r*n + c)*2 + k`, so `dist` is a single `2·m·n` array. This is the
BFS on the **product graph** `G × A`:

- `G` = the grid's 4-neighbour adjacency.
- `A` = a 2-node automaton: node 0 →(spend)→ node 1, node 1 is absorbing.

## Edges (all weight 1)

From `(r, c, k)` to an in-bounds neighbour `(nr, nc)`:

| neighbour cell | condition | resulting state |
|---|---|---|
| open (`1`) | always | `(nr, nc, k)` — stay in layer |
| blocked (`0`) | `k == 0` | `(nr, nc, 1)` — spend the conversion |
| blocked (`0`) | `k == 1` | *no edge* — budget gone |

## The two initialization subtleties

1. **Start layer depends on the start cell.** If `grid[0][0] == 0`, standing on the start
   *is* the conversion, so we seed `(0,0,1)` at distance 0. Otherwise seed `(0,0,0)`.
2. **The goal is two states.** `(m-1,n-1,0)` and `(m-1,n-1,1)` are both accepting. Because BFS
   dequeues in nondecreasing distance, the **first** pop of either is optimal — return
   immediately.

## Correctness sketch

Every edge costs exactly one step and the layer only ever moves `0 → 1`, so any real walk maps
to a path in `G × A` of equal length, and vice versa. BFS on a unit-weight graph returns the
shortest such path; taking the min over both goal layers covers "used the conversion" and
"didn't." Converting a cell you never stand on cannot shorten a path, so restricting
conversions to step-onto moves loses nothing.

## Trace — Example 2 `[[1,0],[0,1]]`

```
seed (0,0,0) d0
 └ (0,1) blocked, k0→1 : (0,1,1) d1
 └ (1,0) blocked, k0→1 : (1,0,1) d1
(0,1,1) d1 └ (1,1) open : (1,1,1) d2   ← goal popped → return 2
```

## Complexity

- **Time / Space:** `O(m·n·2) = O(m·n)`. The constant 2 is the number of automaton states.

## Follow-ups

- **At most `k` conversions.** Replace `A` with a `(k+1)`-state chain (`dist` size `(k+1)·m·n`);
  a blocked step goes `k → k+1` while budget remains. No structural change.
- **Conversions cost extra steps/weight.** Edges are no longer uniform → swap BFS for
  **0-1 BFS (deque)** or Dijkstra over the same product graph.
- **Recover the actual path.** Store a parent per state and backtrack from the goal layer that
  won.
- **Alternative O(mn) method (meet-in-the-middle).** BFS distances from start and from end over
  open cells only; the answer is either `distStart[end]` (no conversion) or, over each blocked
  cell `X`, `min(distStart[a]) + distEnd[b] + 2` for open neighbours `a,b` of `X`. Correct but
  fiddlier on start/end-blocked corners — the product-graph BFS is the safer interview choice.
