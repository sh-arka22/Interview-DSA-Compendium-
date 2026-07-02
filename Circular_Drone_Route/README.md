---
tags:
  - interview
  - prefix-sum
  - array
  - circular
---

# Circular Drone Hub Delivery Route

**Company:** Amazon · **Difficulty:** Medium

## Problem

A drone network is a **ring** of `n` hubs `0..n-1`. `corridor[i]` is the length of the
corridor between hub `i` and hub `(i+1) % n`, so hub `n-1` wraps back to hub `0`. A drone flies
only along corridors, in either direction.

Given `route` — a list of `m` hubs to visit **in order** — the drone starts at `route[0]` and
flies to each next hub. For every leg it takes the **shorter of the two arcs** around the ring
(a tie may go either way, same cost). A leg between identical hubs costs `0`.

Return the total distance flown. **Use 64-bit integers** — the answer can exceed 32-bit range.

### Examples

| `corridor` | `route` | Output | Why |
|---|---|---|---|
| `[1,2,3,4]` | `[0,2,3]` | `6` | `0→2` = min(3, 7) = 3; `2→3` = min(3, 7) = 3. |
| `[1,2,3,4]` | `[1,3]` | `5` | Both arcs from 1 to 3 equal 5. |
| `[4,4,4]` | `[0,1,1,0]` | `8` | `0→1`=4, `1→1`=0 (same hub), `1→0`=4. |

## Core idea — unroll the ring with prefix sums

The two arcs between two hubs always **partition the whole loop**: `arc + other = total`. So we
never need to walk corridor by corridor. Build a prefix-sum array `pre` where `pre[i]` is the
clockwise distance from hub `0` to hub `i` (`pre[0]=0`, `pre[n]=total`). For a leg between hubs
`a` and `b` with `lo = min(a,b)`, `hi = max(a,b)`:

```
clockwise arc (lo → hi) = pre[hi] - pre[lo]
the other arc           = total - (pre[hi] - pre[lo])
leg cost                = min(the two)
```

Each leg is then `O(1)`. Same hub → `lo == hi` → arc `0`, which is naturally the minimum.

## Edge Cases

- **Single stop (`m == 1`)** — no legs, answer `0`. The leg loop simply never runs.
- **Same-hub legs** — consecutive equal entries give `pre[hi]-pre[lo] = 0`; the min picks `0`.
- **Tie between the two arcs** — both equal `total/2`; either is taken, cost is that value
  (Example 2).
- **Wraparound is shorter** — when `hi` is far from `lo` clockwise, the counterclockwise
  (`total - arc`) side wins; the `min` handles it with no special casing (e.g. `[1,2,3,4]`,
  `0→3` costs `4`, not `6`).
- **`n == 2`** — only two hubs; the two arcs are `corridor[0]` and `corridor[1]`. Works
  unchanged.
- **64-bit overflow** — up to `10^5` legs of ~`5·10^8` each ⇒ ~`5·10^13`; `res` and `pre` are
  `long long`. A single `total` (≤ `10^9`) fits in 32-bit, but the running sum does not.
- **Route out of visiting order** — the drone must not reorder stops; the leg loop walks
  `route` strictly left to right.

## Complexity

- **Time:** `O(n + m)` — one pass to build prefixes, one pass over the route.
- **Space:** `O(n)` for the prefix-sum array.
