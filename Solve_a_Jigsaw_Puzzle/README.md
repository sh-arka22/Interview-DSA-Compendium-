---
company: Asana
difficulty: Medium
round: Technical Screen
tags:
  - asana
  - medium
  - backtracking
  - constraint-satisfaction
  - factor-enumeration
algorithms:
  - Backtracking
  - Constraint Satisfaction
  - Precomputed Match Tables
  - Factor Enumeration
---

# Solve a Jigsaw Puzzle

**Company:** Asana | **Role:** Software Engineer | **Difficulty:** Medium | **Round:** Technical Screen

---

## Problem Statement

You are given a collection of **N jigsaw puzzle pieces**. Each piece has four edges — `top`, `right`, `bottom`, `left` — and a helper function `match(edgeA, edgeB)` that returns `true` if two edges can be physically connected.

The pieces form exactly one complete **rectangular grid**, but the number of rows `R` and columns `C` is **not given** — only that `R × C = N`. Pieces may need to be **rotated** by 0°, 90°, 180°, or 270° to fit.

Implement `solve(pieces)` that returns a valid rectangular arrangement of all pieces (with each piece's chosen orientation) such that every pair of adjacent edges matches.

**Guarantees:**
- Every piece must be used exactly once.
- A valid solution is guaranteed to exist.
- Border edges on the outer boundary do not need to match anything.

---

## Rotation Convention

A 90° **clockwise** rotation cyclically permutes edges:

```
Original:  [top, right, bottom, left]
After 90°: [left, top, right, bottom]   (i.e., what was Left is now Top)
```

So the edge at direction `d` after `r` clockwise rotations is:

```
rotatedEdge(piece, r, d) = piece.edges[(d - r + 4) % 4]
```

---

## Constraints & Assumptions

- `N = R × C`; `R, C ≥ 1`; neither is given.
- Edges are opaque — compared **only** through `match`. Do not assume `==`, hashing, or ordering on edge values.
- `match` is O(1) and symmetric: `match(a, b) == match(b, a)`.
- Matching may or may not be one-to-one. An edge could match several others.
- Pieces are distinguishable by identity (index), not by edge values.

---

## Examples

**N = 4 (2 × 2 grid)**

```
Pieces (top, right, bottom, left):
  P0 = [*, 1, 2, *]     P1 = [*, *, 3, -1]
  P2 = [-2, 4, *, *]    P3 = [-3, *, *, -4]

Valid 2×2 layout:
  P0  P1       match(P0.right=1, P1.left=-1)  ✓
  P2  P3       match(P0.bottom=2, P2.top=-2)  ✓
               match(P1.bottom=3, P3.top=-3)  ✓
               match(P2.right=4, P3.left=-4)  ✓
```

(`*` = border edge, not checked)

---

## Edge Cases

| # | Case | Why It Matters |
|---|---|---|
| 1 | **N = 0** | Return empty grid immediately; no pieces to place. |
| 2 | **N = 1** | Trivial 1×1; no adjacency constraints. Any rotation works — return rotation 0. |
| 3 | **N is prime** | Only factor pairs are (1, N) and (N, 1) — must be a straight strip. |
| 4 | **1×N or N×1 strip** | Only horizontal (or only vertical) interior constraints. Top/bottom (or left/right) edges are always border. |
| 5 | **Perfect square N (R = C)** | (R, C) and (C, R) are the same shape; try it only once to avoid duplicate work. |
| 6 | **Multiple valid grid shapes** | N = 4 allows (1×4), (2×2), (4×1). Solver must try each and stop at first success. |
| 7 | **Piece requires non-zero rotation** | Never assume rotation 0 is always correct. All 4 rotations must be tried at each cell. |
| 8 | **4-fold symmetric piece** (all edges identical) | All 4 rotations are equivalent — solver will try all 4 but they succeed/fail identically. Doesn't break correctness, but wastes work. |
| 9 | **All pieces have the same edge on all sides** | Every piece can be neighbor to every other piece. Backtracking may explore many branches. |
| 10 | **Bijective matching** (each edge matches exactly one other) | Highly selective — effective branching factor ≈ 1 per cell. Near O(N) in practice. |
| 11 | **Ambiguous matching** (one edge matches many) | Increases branching factor drastically. Pruning via precomputed tables is critical. |
| 12 | **Factor pair order matters for performance** | Squarish grids (R ≈ C) give more constraints per cell sooner, pruning more. Try them before extreme strips. |
| 13 | **Integer overflow in factor loop** | `r * r ≤ N` overflows int for large N. Use `(long long)r * r`. |
| 14 | **Self-adjacent edge** | If a piece's right edge could match its own left edge, `used[]` tracking must still prevent placing the same piece twice. |
| 15 | **Grid dimensions with 6-piece puzzle** | N=6 admits (1×6), (6×1), (2×3), (3×2). A 1×6 strip may be impossible even though 2×3 works — solver must exhaust each shape before moving on. |

---

## Clarifying Questions to Ask

1. Is the solution unique, or may there be multiple valid arrangements?
2. Can a single edge be compatible with many other edges, or is `match` close to a bijection?
3. Are edges representable as comparable/hashable values, or only through `match`?
4. For border detection: is there a sentinel value for "border edge," or do we infer from grid position?
5. What is the expected scale of N — dozens, thousands, or millions?
6. Should the output include absolute position and rotation, or just relative adjacency?

---

## Follow-up Questions

- If `match` is expensive (e.g., a vision/network call), how do you minimize calls? (Cache, precompute.)
- How would you parallelize or distribute the search across cores/machines?
- What if the puzzle is not guaranteed solvable, or some pieces are missing?
- How do you adapt if pieces can also be **flipped** (mirrored), not just rotated?

---

## Algorithm

See [ALGORITHM.md](./ALGORITHM.md) for the full breakdown.

**Algorithms Used:** Backtracking · Constraint Satisfaction · Precomputed Match Tables · Factor Enumeration
