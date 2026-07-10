---
tags:
  - interview
  - graph
  - shortest-path
  - 0-1-bfs
  - dijkstra
  - binary-search
---

# Minimum Cells to Bridge a Magic Grid

**Company:** Apple · **Difficulty:** Hard

## Problem

Grid `runes` (`rows x cols`), each cell one of: `0` empty, `1` charged, `2` forbidden.
Magic flows between 4-directionally adjacent **charged** cells. A spell is castable when a
charged path connects row `0` to row `rows - 1`. Charging an empty cell costs 1 rune;
forbidden cells can never be charged or traversed.

Return the **minimum number of empty cells to charge** so the spell is castable, `0` if
already castable, `-1` if impossible.

### Examples

| Input | Output | Why |
|---|---|---|
| `[[1,0,0],[0,0,0],[0,0,1]]` | `2` | Charge (1,0) and (2,0): path (0,0)→(1,0)→(2,0). |
| `[[1,1,1],[2,2,1],[1,1,1]]` | `0` | (0,2)→(1,2)→(2,2) already charged. |

Constraints: `1 ≤ rows, cols ≤ 1000`, `rows·cols ≤ 10^6`.

## The reframe — shortest path on a weighted grid

"Minimum cells to charge along some route" is a **shortest path** in disguise. Price
*entering* a cell: `1` if empty (you must charge it), `0` if charged (free ride), wall if
forbidden. Add a virtual source above row 0 connected to every non-forbidden row-0 cell
(paying that cell's own price — endpoints cost 1 too, the most common bug). The answer is
the min distance to any bottom-row cell.

The `rows == 1` note in the statement dissolves: the multi-source init already prices row-0
cells, and row 0 is also the bottom row.

## Three implementations (all in this folder, cross-verified)

| File | Approach | Complexity | 1000×1000 bench |
|---|---|---|---|
| `solution.cpp` ★ | **0-1 BFS** (deque) | **O(RC)** | 38 ms |
| `solution_dijkstra.cpp` | Dijkstra (min-heap) | O(RC log RC) | 117 ms |
| `solution_binary_search.cpp` | Binary search on answer + layered DFS | O(RC log RC) | 918 ms |

★ = what to write in a 30-minute interview: asymptotically optimal, shortest to code, and
shows you recognise the 0/1-weight specialization. Full derivations in `ALGORITHM.md`.

## Edge Cases

- **Endpoints cost too** — an empty cell in row 0 or row rows-1 must be paid for.
- **`rows == 1`** — `[[0]]` → `1`, `[[1]]` → `0`, `[[2]]` → `-1`. No special-case code.
- **Diagonals never connect** — `[[1,2],[2,1]]` → `-1`.
- **Solid forbidden wall** — `-1` regardless of budget.
- **Already castable** — return `0`; the winding path `[[1,2,2],[1,1,1],[2,2,1]]` → `0`.
- **Empty entry can be re-relaxed** — 0-1 BFS deque holds duplicates; skip cells already
  finalized (`done` array), exactly like Dijkstra's lazy deletion.

## Run

```bash
g++ -std=c++17 -O2 -o test test.cpp && ./test
```
