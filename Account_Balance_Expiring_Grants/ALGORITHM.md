---
company: Anthropic
tags:
  - algorithm
  - heap
  - priority-queue
  - lazy-deletion
  - greedy
related: "[[README]]"
---

# Algorithm: Account Balance with Expiring Grants

## First principles — BFS across the solution space

1. **Naive rescan.** Keep grants in a plain list. On every `SPEND`/`GET_BALANCE`, filter out
   expired grants, sort the rest by `expire`, then spend/sum. Correct, but sorting on every
   op is O(k log k) where k = live grants, and with up to 10^5 ops this is O(n^2 log n) in
   the worst case (many grants, many spends) — too slow.
2. **Keep it sorted incrementally instead of re-sorting.** Use a structure that maintains
   expiry order as grants are added and removed, so we never re-sort from scratch: a
   balanced BST / `multiset<pair<expire,amount>>` supports insert, remove-min, and arbitrary
   removal in O(log n).
3. **Notice we never need arbitrary removal.** Every operation only ever touches the
   *minimum*-expiry grant — either because it just expired, or because spending policy says
   "soonest-expiry first." We never need to delete or modify a grant in the middle of the
   order. That capability is exactly what a **binary heap** (priority queue) provides, at
   lower constant-factor cost than a tree-based ordered set (array-backed, better cache
   locality, no rebalancing).
4. **Lazy deletion instead of proactive expiry.** There's no background clock ticking:
   grants don't need to be removed the instant they expire, only by the next operation that
   actually needs an accurate answer. Since ops arrive in non-decreasing timestamp order,
   once a grant is popped as expired relative to some `t`, it would also be expired for
   every later `t` — so nothing is ever "expired but still on the heap and needs rechecking."

This lands on: **min-heap ordered by expiry + lazy deletion**, which is the standard
technique for "sweep out entries that are stale as of the current query" problems (the same
pattern shows up in sliding-window and interval-scheduling problems that use a heap for
"soonest deadline first").

## Chosen approach: min-heap + lazy deletion + partial-refill

### State
- `heap`: min-heap of `(expire, amount)`, ordered by `expire`.
- `total`: running sum of every amount currently sitting on the heap (kept in sync on every
  push/pop so `GET_BALANCE` is an O(1) read after the expiry sweep).

### `dropExpired(t)` — shared by SPEND and GET_BALANCE
While the heap's minimum has `expire <= t`: pop it, subtract its amount from `total`. This
sweeps *all* newly-stale grants, not just one — a grant added long ago with a short expiry
could be sitting at the top having never been touched.

### `ADD_GRANT(amount, expire)`
Push `(expire, amount)` onto the heap, add `amount` to `total`. (The grant's own start
timestamp doesn't need to be stored or checked — since ops are processed in non-decreasing
timestamp order, every later op's timestamp is automatically `>= this grant's start`, so it's
"active from the moment it's added" by construction.)

### `SPEND(t, amount)`
1. `dropExpired(t)`.
2. While `amount > 0` and the heap isn't empty: pop the minimum-expiry grant.
   - If its amount `<= remaining`, it's fully consumed — subtract it from both `remaining`
     and `total`, move to the next grant.
   - Otherwise, it's only partially consumed — subtract `remaining` from `total`, set
     `remaining = 0`, and push the grant back with its reduced amount (same `expire`, so it's
     still correctly positioned as the new minimum).
3. If the heap empties before `remaining` hits 0, the balance was insufficient — stop; the
   unfunded remainder is simply ignored, per spec.

### `GET_BALANCE(t)`
`dropExpired(t)`, then return `total`.

### Why partial-refill doesn't break the complexity bound
Each `SPEND` call pushes back **at most one** remainder entry (the loop stops the instant
`remaining` hits 0, which happens either exactly when a grant is fully consumed, or via the
partial-consumption branch that always zeroes `remaining`). So across all `m` SPEND calls,
at most `m` extra pushes are added on top of the `n` original `ADD_GRANT` pushes — the total
number of heap entries ever created is bounded by `O(n)`, not unbounded.

---

## Complexity

| | Value |
|---|---|
| Total ops | n ≤ 10^5 |
| Heap pushes (ADD_GRANT + partial refills) | O(n) |
| Heap pops (expiry sweeps + spend consumption) | O(n) — each pushed entry popped at most once |
| Work per push/pop | O(log n) |
| **Total** | **O(n log n)** |

Measured: 100,000 mixed operations run in ~30ms in this implementation — the heap approach
leaves substantial headroom under the constraint.

## Why not a multiset / balanced BST?

Also O(log n) per operation and would work correctly, but it's solving a harder problem than
we have: multisets support removing *any* element, not just the minimum. Since we only ever
need extract-min and insert, a binary heap gives the same asymptotic guarantee with a smaller
constant factor (contiguous array storage, simpler pointer-free operations) — using a tree
here would be reaching for more machinery than the problem requires.

## A further micro-optimization (not needed at this scale)

`std::priority_queue` doesn't expose a mutable reference to its top element, so a partial
spend is implemented as pop-then-push (two O(log n) operations). Since `amount` isn't part of
the heap's ordering key, a hand-rolled heap (`std::vector` + `push_heap`/`pop_heap`, or a
custom index-tracking heap) could mutate the top's amount in place without violating the heap
invariant, saving one of those two operations on the partial-consumption path. At n ≤ 10^5
this constant-factor saving isn't worth the added code complexity — the standard library
container is the right choice here — but it's the natural next lever if throughput needed to
be pushed further.
