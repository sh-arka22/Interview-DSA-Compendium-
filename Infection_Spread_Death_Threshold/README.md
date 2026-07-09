---
tags:
  - interview
  - simulation
  - cellular-automaton
  - grid
---

# Infection Spread Simulation with Death Threshold

**Company:** OpenAI · **Difficulty:** Medium

## Problem

An `m x n` grid simulates an infection over discrete days. Each cell is one of four states:

- `0` — uninfected (susceptible)
- `1` — infected
- `2` — immune (recovered; can never be infected again — permanent)
- `3` — dead (permanent)

Neighbors are the up to **8** Moore-neighborhood cells (orthogonal + diagonal). Cells off the
grid are not neighbors. Two integer parameters drive the dynamics:

- **`infectThreshold`** — a `0` cell becomes `1` next day if **at least** `infectThreshold` of its
  neighbors are infected on the current day.
- **`deathThreshold`** — at the instant a cell becomes infected, if **at least** `deathThreshold`
  of its neighbors are also infected, the cell is permanently **marked for death**.

Every infected cell stays infected for **exactly one full day**, then resolves the next day:
immune (`2`) if it was not marked, dead (`3`) if it was.

### Transition rules (applied simultaneously — every decision reads the *current* day)

| Current | Next |
|---|---|
| `0` uninfected | Let `c` = infected neighbors. `c >= infectThreshold` → `1` (and if `c >= deathThreshold`, mark it for death); else stays `0`. |
| `1` infected | `3` if marked for death, else `2`. |
| `2` immune | `2` forever. |
| `3` dead | `3` forever. |

A cell that becomes infected uses the **same** neighbor count `c` for both the infection check
(`>= infectThreshold`) and the death-mark check (`>= deathThreshold`). Cells infected in the
**initial** grid are treated as **not** marked for death. Run until the grid reaches a **stable
state** — a day on which applying the rules reproduces the current grid — and return it.

### Examples

| `grid` | `infect` | `death` | Output | Why |
|---|---|---|---|---|
| `[[1,0,0],[0,0,0],[0,0,0]]` | `1` | `1` | `[[2,3,3],[3,3,3],[3,3,3]]` | Seed recovers to `2` (no infected neighbors, unmarked). Everything it infects has `c=1 >= death=1`, so all are marked and die. |
| `[[0,0,0],[0,1,0],[0,0,0]]` | `1` | `2` | `[[2,2,2],[2,2,2],[2,2,2]]` | Center recovers to `2`. Each ring cell has exactly `1` infected neighbor: `1 >= infect` so it infects, but `1 < death` so it is **not** marked — all recover. |

## Core idea — event-driven frontier

The naive automaton re-sweeps all `m·n` cells every day. But every cell moves strictly
**forward** through `0 → infected → immune/dead`, so **each cell is infected at most once**. That
lets us drive the simulation off a **frontier** — the cells infected *today* — instead of
scanning the whole grid:

1. **Spread.** Each frontier cell casts one vote to every `0`-neighbor, tallied in a scratch grid
   `cnt`. A `0`-cell whose tally reaches `>= infectThreshold` joins **tomorrow's** frontier; at
   that same instant, if the tally is also `>= deathThreshold` the cell is **marked for death**
   (one count `c` drives both checks).
2. **Resolve.** Today's frontier then flips to `2` (immune) or `3` (dead) per its death mark.
3. Set the newly infected cells to `1` and repeat. When the frontier empties, no further
   infection is possible — the grid is stable and we return it.

**Simultaneity** is preserved by ordering: every vote is counted *before* any frontier cell is
resolved and *before* any new cell is set to `1`, so tallies only ever see the current day's
infected set. **Initial-grid infecteds are seeded into the frontier unmarked** — only a
`0 → infected` transition can set a death mark. The `cnt` grid is allocated once and reused: each
round remembers which cells it `touched` and zeroes exactly those, so clearing costs
`O(touched)`, never a full `O(m·n)` wipe.

## Edge Cases

- **Initial infected are unmarked.** A `1` in the input resolves to immune (`2`), never dead —
  even when its neighbors would mark a *freshly* infected cell. Initial cells are only seeded
  into the frontier; the death mark is set solely on a `0 → infected` transition. Both
  `[[1]] → [[2]]` and `[[1,1]], 1, 1 → [[2,2]]` (two adjacent seeds still recover).
- **Same `c` for both checks.** Infection and death-mark read one tally in the same branch — a
  cell can never be marked-for-death without also being infected.
- **Below `infectThreshold`.** A lone infected neighbor with `infectThreshold = 2` never spreads;
  the infection dies out and susceptibles stay `0` (`[[0,1,0]], 2, 1 → [[0,2,0]]`).
- **No infected cells / already stable.** The frontier starts empty, the loop never runs, and the
  grid is returned unchanged (covers all-`0` and all-`2/3` inputs).
- **`immune`/`dead` are absorbing.** Votes only target `0`-cells (`grid[nx][ny] != 0` is skipped),
  so a `2` is never reinfected and a `3` never changes.
- **Grid boundaries.** Off-grid cells are not neighbors; corners have 3 neighbors, edges 5. Bounds
  are checked before every neighbor read — no padding, no wraparound.
- **A touched-but-not-infected `0`-cell.** Its round tally is discarded (reset to `0`), so next
  round it is re-evaluated fresh against the *new* frontier — matching per-day independence.

## Complexity

- **Time:** `O(m · n)`. Each cell is infected at most once, and each infection event does `O(8)`
  vote work; per-round `touched` processing is charged to the votes that created it. Seeding and
  never-touched cells are `O(m · n)`. **No dependence on the day count** — the win over a per-day
  full sweep (`O((m + n) · m · n)`).
- **Space:** `O(m · n)` for `cnt`, `markedForDeath`, and the frontier lists.
