# Algorithm — Prefix Sums on a Ring

## The observation that makes it O(1) per leg

On a ring, the two ways to fly from hub `a` to hub `b` are the two arcs of the circle, and they
**always sum to the full loop length** `total`. Knowing one arc gives the other for free. So the
whole problem reduces to: *for each leg, compute one arc quickly, then take `min(arc, total - arc)`.*

## Prefix sums as unrolled distances

Cut the ring open between hub `n-1` and hub `0` and lay it flat. Define

```
pre[0] = 0
pre[i] = corridor[0] + corridor[1] + ... + corridor[i-1]      (i = 1..n)
pre[n] = total  (the full loop)
```

`pre[i]` is the clockwise distance from hub `0` to hub `i`. For any two hubs, the clockwise arc
from the lower-indexed to the higher-indexed one is a single subtraction:

```
lo = min(a, b),  hi = max(a, b)
arc      = pre[hi] - pre[lo]      // clockwise, lo -> hi (the arc NOT crossing the 0-seam)
otherArc = total - arc            // the arc that crosses the seam
cost     = min(arc, otherArc)
```

Ordering the endpoints (`lo`, `hi`) is what lets us always name the seam-free arc as a positive
difference; direction of travel is irrelevant since crossing a corridor costs the same both ways.

## Why ordering the endpoints is enough (no modular arithmetic)

A common alternative is `arc = (pre[b] - pre[a] + total) % total`. That also works, but by taking
`lo`/`hi` first we guarantee `pre[hi] - pre[lo] >= 0`, so no modulo and no negative intermediate.
Both arcs are then non-negative and sum to `total`, so `min` is exactly the shorter path.

## Correctness sketch

- `pre[hi] - pre[lo]` telescopes to `corridor[lo] + ... + corridor[hi-1]`, precisely the
  clockwise arc that stays between the two hubs — one of the two real routes.
- Its complement `total - arc` is the other real route (over the seam). Every physical path
  between the hubs is one of these two, so their `min` is optimal for the leg.
- Same hub ⇒ `lo == hi` ⇒ `arc = 0 = min`, matching the "cost 0" rule.
- Legs are independent and summed, matching "visit in the given order."

## Trace — Example 1, `corridor=[1,2,3,4]`, `route=[0,2,3]`

```
pre = [0, 1, 3, 6, 10],  total = 10
leg 0->2 : lo0 hi2  arc = pre[2]-pre[0] = 3   min(3, 7) = 3
leg 2->3 : lo2 hi3  arc = pre[3]-pre[2] = 3   min(3, 7) = 3
res = 6
```

## Complexity

- **Time:** `O(n)` to build `pre` + `O(m)` for the legs = `O(n + m)`.
- **Space:** `O(n)` for `pre` (an in-place running variable can drop it to `O(1)` only if the
  route is processed alongside the build — not worth it; `pre` keeps legs random-access `O(1)`).

## Follow-ups

- **Queries instead of a route.** With `pre` precomputed, answer any "shortest distance between
  hub `a` and `b`" in `O(1)`; the route version is just `m-1` such queries chained.
- **Directional / one-way corridors** (clockwise ≠ counterclockwise cost). Keep two prefix
  arrays, one per direction; each leg compares `preCW` vs `preCCW` explicitly.
- **Corridors change over time (updates).** Swap the static prefix array for a **Fenwick/BIT**
  so a corridor update and an arc query are both `O(log n)`.
- **Full shortest path, not just distance.** Record which arc won per leg to reconstruct the
  actual hub sequence flown.
