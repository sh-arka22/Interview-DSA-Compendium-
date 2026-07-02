---
tags:
  - interview
  - bfs
  - graph
  - shortest-path
---

# Minimum Path Length Through a Grid With One Allowed Cell Conversion

**Company:** Amazon · **Difficulty:** Medium

## Problem

Given an `m x n` grid where `1` = open (you may stand on it) and `0` = blocked. Start at
`(0,0)`, reach `(m-1, n-1)`, moving one step up/down/left/right onto open cells only.

**Before/while walking you may convert at most one `0` into a `1`** — any single cell,
including the start or end. A cell you ever stand on must be open (originally, or made open by
your one conversion). Return the **minimum number of steps**, or `-1` if unreachable even with
the conversion.

### Examples

| Input | Output | Why |
|---|---|---|
| `[[1,1,0],[0,1,0],[0,1,1]]` | `4` | An all-open path already exists; conversion unused. |
| `[[1,0],[0,1]]` | `2` | Both neighbours blocked; convert one to slip through. |

## The innovative idea — a product graph

The naive instinct is to bolt a "have I converted yet?" boolean onto plain BFS. The cleaner
reframe: search on **`grid × A`**, where `A` is a 2-state automaton tracking the conversion
budget.

```
        layer 0 (conversion available)          layer 1 (conversion spent)
        ┌───┬───┬───┐                            ┌───┬───┬───┐
        │   │   │   │   step onto a BLOCKED      │   │   │   │
        │   │   │   │  ─────cell (0→1)────►      │   │   │   │
        └───┴───┴───┘   (the ONLY inter-layer    └───┴───┴───┘
     open↔open moves    edge; costs the budget)   open↔open moves
       stay in layer 0                              stay in layer 1
```

- Moving onto an **open** cell stays in the same layer.
- Moving onto a **blocked** cell is legal only from layer 0 and is exactly the edge that drops
  you into layer 1 (you never leave layer 1).
- Every edge costs one step, so a single BFS over the doubled state space gives the shortest
  path; the answer is the first time BFS pops the goal in **either** layer.

Why this is nice: the special-case rule "one conversion" dissolves into "a second copy of the
grid." Bump `A` to `k+1` states and the same code solves **at most `k` conversions** — no new
logic. See `ALGORITHM.md`.

## Edge Cases

- **Blocked start `(0,0)`** — you must spend the conversion just to stand on it, so the search
  begins already in layer 1. (`[[0]]` → `0`: convert the start, you're already at the goal.)
- **Blocked goal** — the conversion may be spent on the destination itself (`[[1,1],[1,0]]` → `2`).
- **Conversion unnecessary** — an all-open shortest path wins; budget is simply never used.
- **Two barriers** — one hole is not enough → `-1` (`[[1,0,0],[0,0,0],[0,0,1]]`).
- **1×1 grid** — start == goal → `0` steps (open or converted).
- Converting a cell you never stand on is pointless, so only "step-onto" conversions matter —
  precisely the inter-layer edge above.

## Complexity

- **Time:** `O(m · n · 2)` — every (cell, layer) state enqueued once, `O(1)` work each.
- **Space:** `O(m · n · 2)` for the distance/visited array and the queue.
