# Algorithm — Event-Driven Frontier Simulation

## The key observation

Every cell moves strictly forward through its life cycle:

```
0 (uninfected) ---> 1 (infected, exactly one day) ---> 2 (immune)  or  3 (dead)
                                                        \__ both permanent __/
```

So **each cell is infected at most once**, and `2`/`3` are absorbing. A full-grid re-sweep every
day therefore wastes almost all its work on cells that can never change again. We instead track
only the **frontier** — the cells infected *today* — and let infection events, not days, drive the
cost.

## State we carry

- `grid` — mutated in place to the answer (`0/1/2/3`).
- `frontier` — list of the cells infected on the current day.
- `markedForDeath[i][j]` — whether an infected cell resolves to dead vs. immune.
- `cnt[i][j]` — a reusable scratch grid holding, for the current round, how many infected
  neighbors a `0`-cell has seen so far.

## One round

```
1. SPREAD   for each cell (x,y) in frontier:
              for each of its 8 neighbors (nx,ny) that is currently 0:
                if cnt[nx][ny] == 0: remember (nx,ny) in `touched`   // first vote this round
                cnt[nx][ny]++                                        // one vote

2. DECIDE   for each (x,y) in touched:
              if cnt[x][y] >= infectThreshold:
                add (x,y) to nextFrontier
                markedForDeath[x][y] = (cnt[x][y] >= deathThreshold) // same tally, both checks
              cnt[x][y] = 0                                          // discard tally -> ready to reuse

3. COMMIT   for each (x,y) in frontier:      grid[x][y] = markedForDeath[x][y] ? 3 : 2  // resolve
            for each (x,y) in nextFrontier:  grid[x][y] = 1                             // infect

4. frontier = nextFrontier
```

Loop while the frontier is non-empty. An empty frontier means no `0`-cell reached the infection
threshold, so nothing can change again — the fixed point.

## Why simultaneity holds

All transitions must read the **same** snapshot. The round enforces that purely by ordering:

- **Counting happens first** (step 1), while every frontier cell is still `1` and no new cell is
  yet `1`. So a `0`-cell's tally counts exactly the infected cells of the *current* day — not a
  neighbor that turns infected later this same round, and not a frontier cell that has already
  resolved.
- Only after all tallies are fixed do we **resolve** the old frontier and **infect** the new one
  (step 3). A newly infected cell cannot influence anyone until it appears in `frontier` next
  round.

This is the frontier equivalent of double-buffering, but it touches only active cells.

## Initial infected cells are *not* marked

The spec: *"Cells infected in the initial grid are treated as not marked for death."* We honor it
by simply seeding those cells into `frontier` with `markedForDeath = false`. The death mark is set
in exactly one place — step 2, a `0 → infected` transition — so an initial `1` can only resolve to
immune (`2`). Example: `[[1,1]]`, `infect=1`, `death=1` → both seeds have an infected neighbor but
are unmarked → `[[2,2]]`, not `[[3,3]]`.

## Reusing `cnt` without an O(m·n) clear

`cnt` is allocated once. Zeroing the whole grid each round would reintroduce the `O(m·n)`-per-day
cost we are trying to avoid. Instead each round records every cell it bumped in `touched`, and step
2 resets exactly those back to `0`. Since a `0`-cell only lands in `touched` when its counter goes
`0 → 1`, every touched cell is cleared exactly once — the grid is provably all-zero again before
the next round, at cost proportional to the votes actually cast.

## Correctness sketch

- A `0`-cell's tally after step 1 equals its true number of infected (frontier) neighbors, because
  every infected neighbor votes once and nothing else does.
- It becomes infected iff that tally `>= infectThreshold`, and is marked iff the *same* tally
  `>= deathThreshold` — exactly the spec's "same `c` for both checks."
- Infected cells resolve after their single day to `3` (marked) or `2` (unmarked) — step 3.
- `2`/`3` are never revisited: votes target only `0`-cells, and infected cells are handled by the
  frontier itself. The loop halts when no new infection occurs, which is the stable state.

## Trace — Example 1, `grid=[[1,0,0],[0,0,0],[0,0,0]]`, `infect=1`, `death=1`

```
frontier {(0,0)}   seed, unmarked
Round 1 votes: (0,1),(1,0),(1,1) each get 1 vote (>=infect, >=death -> marked)
        resolve (0,0)->2 ; infect the three -> 1
        grid: 2 1 0 / 1 1 0 / 0 0 0
Round 2 votes from {(0,1),(1,0),(1,1)}: (0,2)=2,(1,2)=2,(2,0)=2,(2,1)=2,(2,2)=1 (all>=death->marked)
        resolve the three -> 3 ; infect the five -> 1
        grid: 2 3 1 / 3 3 1 / 1 1 1
Round 3 votes from those five: no remaining 0-cells -> nextFrontier empty
        resolve the five -> 3
        grid: 2 3 3 / 3 3 3 / 3 3 3      frontier empty -> stable
```

## Complexity

- **Time:** `O(m · n)`. Each cell is infected at most once; each infection event does `O(8)` vote
  work, and each round's `touched` bookkeeping is charged to the votes that produced it. Seeding
  the frontier and the cells never touched are `O(m · n)`. Crucially there is **no factor of the
  day count** — unlike a per-day full sweep at `O((m + n) · m · n)`.
- **Space:** `O(m · n)` for `cnt`, `markedForDeath`, and the frontier lists (whose sizes sum to
  ≤ `m · n` over the whole run).

## Follow-ups

- **Variable infectious period.** If a cell stayed infected `k` days, replace the single-day
  resolve with a per-cell countdown before it moves to `2`/`3`.
- **Waning immunity / reinfection.** Let `2` decay back to `0` after some days; the system no
  longer reaches an absorbing fixed point, so you would detect a repeating cycle instead of an
  empty frontier.
- **Streaming huge grids.** `cnt` and `markedForDeath` are `O(m·n)`; for grids too large to hold
  two companion arrays, fall back to a row-window sweep (`O(n)` extra) trading time for space.
- **Parallelism.** The per-round vote/decide phase is embarrassingly parallel over frontier cells;
  the only care needed is atomic increments on shared `cnt` (or per-thread tallies merged).
