# Algorithm: Solve a Jigsaw Puzzle

## High-Level Strategy

The problem has **two unknowns**: the grid shape (R × C) and each piece's rotation. Resolve them separately:

1. **Enumerate factor pairs** (R, C) of N — O(√N) candidates.
2. For each shape, run **backtracking in row-major order** with precomputed match tables.
3. Return the first valid arrangement found.

---

## Phase 1 — Grid Shape Enumeration

Iterate `r` from 1 up to √N. For each `r` that divides N:
- Try shape (r, N/r).
- If r ≠ N/r, also try shape (N/r, r).
- Stop as soon as backtracking succeeds.

```
for r in 1..√N:
    if N % r != 0: continue
    c = N / r
    if backtrack(r, c) succeeds: return result
    if r != c and backtrack(c, r) succeeds: return result
```

**Why squarish first?** Shapes where R ≈ C give more constraints per cell sooner (both a top and a left neighbor), which prunes branches faster. The iteration order `r=1,2,...,√N` naturally tries strips first and square last — for performance you could reverse this, but correctness is unaffected.

---

## Phase 2 — Precomputation

Since `match` is the only oracle and edges are opaque, we precompute all pairwise compatibility up front to avoid repeated O(1) calls during backtracking.

**Oriented piece index:** encode `(piece_idx, rotation)` as `piece_idx * 4 + rotation` (range 0..4N-1).

Precompute two tables:

| Table | Meaning |
|---|---|
| `preH[a][b]` | Right edge of oriented piece `a` matches left edge of oriented piece `b` |
| `preV[a][b]` | Bottom edge of oriented piece `a` matches top edge of oriented piece `b` |

```
for each (i, ri) — piece i at rotation ri:
    for each (j, rj) — piece j at rotation rj (j ≠ i):
        preH[i*4+ri][j*4+rj] = match(rotEdge(i, ri, RIGHT),  rotEdge(j, rj, LEFT))
        preV[i*4+ri][j*4+rj] = match(rotEdge(i, ri, BOTTOM), rotEdge(j, rj, TOP))
```

**Cost:** 16N² match() calls once. After that, all constraint checks are O(1) array lookups.

---

## Phase 3 — Backtracking

Fill the R×C grid in **reading order** (row-major). At position `pos`:
- `r = pos / C`, `c = pos % C`
- If `r > 0`: must satisfy vertical match with `grid[r-1][c]`
- If `c > 0`: must satisfy horizontal match with `grid[r][c-1]`
- Otherwise that direction is a border — no constraint.

```
backtrack(pos, R, C):
    if pos == N: return true   // all pieces placed successfully

    r, c = pos / C, pos % C
    topKey  = (r > 0) ? oriented_key(grid[pos - C]) : -1
    leftKey = (c > 0) ? oriented_key(grid[pos - 1]) : -1

    for each unused piece i:
        for rot in {0, 1, 2, 3}:
            key = i * 4 + rot
            if topKey  >= 0 and not preV[topKey][key]:  skip
            if leftKey >= 0 and not preH[leftKey][key]: skip

            place (i, rot) at pos
            mark i as used
            if backtrack(pos + 1, R, C): return true
            unmark i
    
    return false
```

**Undo step is critical** — if recursion fails, `used[i]` must be cleared so `i` can be tried at a later position.

---

## Rotation Formula

```
rotatedEdge(piece, rotation r, direction d) = piece.edges[(d - r + 4) % 4]
```

**Derivation:**
```
Original piece layout:  [top(0), right(1), bottom(2), left(3)]

After 1 CW rotation:    [left,   top,      right,    bottom]
  → new[0] = old[3], new[1] = old[0], new[2] = old[1], new[3] = old[2]
  → new[d]  = old[(d - 1 + 4) % 4]

After r CW rotations:   new[d] = old[(d - r + 4) % 4]
```

Verification:
| rotation r | direction d | formula | result |
|---|---|---|---|
| 0 | 0 (top) | e[(0-0+4)%4] = e[0] | top ✓ |
| 1 | 0 (top) | e[(0-1+4)%4] = e[3] | left (became top after 90° CW) ✓ |
| 1 | 1 (right) | e[(1-1+4)%4] = e[0] | top (became right after 90° CW) ✓ |
| 2 | 1 (right) | e[(1-2+4)%4] = e[3] | left (became right after 180°) ✓ |

---

## Key Data Structures

| Structure | Type | Purpose |
|---|---|---|
| `preH[4N][4N]` | `bool[][]` | Horizontal match lookup |
| `preV[4N][4N]` | `bool[][]` | Vertical match lookup |
| `flat[N]` | `PlacedPiece[]` | Flat row-major grid (piece_idx, rotation) |
| `used[N]` | `bool[]` | Which pieces have been placed |

---

## Complexity Analysis

### Precomputation
- **Time:** O(16N²) = O(N²) match() calls
- **Space:** O(16N²) = O(N²) for the two tables

### Backtracking
- **Factor pairs enumerated:** O(√N)
- **Stack depth:** O(N) (one frame per cell)
- **Per-cell work:** O(1) per (piece, rotation) candidate via table lookup

| Scenario | Effective branching | Total work |
|---|---|---|
| Bijective matching (each edge matches ≤ 1 other) | ≈ 1–4 per cell | ≈ O(N) |
| k-compatible (each edge matches ≤ k others) | ≤ 4k per cell | O((4k)^N) |
| Fully ambiguous (match always true) | 4N per cell | O((4N)^N) — adversarial |

The guarantee that a solution exists means backtracking will succeed at the correct grid shape; it does **not** bound how many wrong branches are explored before that.

### Space
- O(N²) dominant (precomputed tables)
- O(N) for grid, used array, and call stack

---

## Pruning Strategies

1. **Precomputed tables** — eliminate repeated match() calls; O(1) per candidate check.
2. **Row-major order** — each new cell is constrained by already-placed top and left neighbors. Placing left-to-right means the first row builds the "header" for all column constraints.
3. **Factor pair ordering** — try squarish shapes first for denser constraint networks.
4. **Corner anchoring (optional)** — if border edges can be detected (e.g., have a special sentinel value), restrict the top-left cell to pieces with two "border" edges. Cuts first-cell branching from 4N to much fewer.
5. **Early shape termination** — if the first row fails to complete after exhausting all candidates, abandon this (R, C) pair immediately.

---

## Why Row-Major Order?

An alternative is to try all N! × 4^N placements globally. Row-major order converts this global search into a **chain of local decisions**: each cell has at most 2 constraints (from already-placed neighbors). This makes pruning happen at every step rather than only at the end.

---

## match() Call Count

| Phase | Calls |
|---|---|
| Precomputation | 16N² (once) |
| Backtracking | 0 (uses precomputed tables) |
| **Total** | **16N²** |

If match() is expensive, this is exactly the right trade-off: pay 16N² calls up front to reduce backtracking to pure array lookups.
