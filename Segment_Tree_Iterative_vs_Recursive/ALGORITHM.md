# Segment Tree: Iterative (Bottom-Up) vs Recursive (Top-Down)

## First principles: what problem does a segment tree solve?

You have an array `a[0..n-1]` and need two operations, both fast:

- `query(l, r)` — combine (sum/min/max/gcd/...) over a range
- `update(pos, val)` — change one element

Prefix sums give O(1) query but O(n) update. A plain array gives O(1) update but
O(n) query. The segment tree is the compromise: precompute the answer for O(n)
carefully chosen ranges (halves, quarters, ...) so that **any** range is the
disjoint union of O(log n) of them. Both ops become O(log n).

Any **associative** operation works. Commutativity is NOT required (but see the
iterative caveat below).

---

## The two implementations

### Recursive (top-down)

Node `v` explicitly owns interval `[lo, hi]`; children own the two halves.
Start at the root, recurse only into children that intersect the query.

```
query(v, lo, hi, l, r):
    disjoint        -> return identity
    fully covered   -> return t[v]
    else            -> combine(left child, right child)
```

### Iterative (bottom-up)

No intervals stored at all. One array `t` of size `2n`:

- Leaf `i` of the array lives at `t[n + i]`
- Parent of node `i` is `i >> 1`, sibling is `i ^ 1`, root is `t[1]`

**Update** starts at the leaf and walks parent pointers to the root — a single
for-loop. **Query** starts at the two leaf positions `l+n` and `r+n` and walks
both up simultaneously, absorbing a node whenever the pointer is a "border"
child that its parent would over-cover:

```cpp
long long query(int l, int r) {            // [l, r)
    long long res = 0;
    for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
        if (l & 1) res += t[l++];          // l is a right child: take it, skip past
        if (r & 1) res += t[--r];          // r is a right child: take node left of it
    }
    return res;
}
```

**Invariant:** at every level, the elements between pointers `l` and `r` are
exactly the not-yet-covered part of the query. If `l` is a right child (`l & 1`),
its parent also covers elements *left* of the query, so we must consume `t[l]`
now and move on (`l++`). Symmetric for `r`. Moving up a level (`l>>=1, r>>=1`)
is safe precisely because after those two ifs, both pointers are left/right-
aligned with parent boundaries.

---

## Accurate complexity comparison

| | Iterative (bottom-up) | Recursive (top-down) |
|---|---|---|
| Build | **O(n)** — exactly `n − 1` combines, one linear pass | O(n) — ~`2n − 1` nodes, but via `2n − 1` function calls |
| Point update | **Θ(log n)** — exactly `⌊log₂ n⌋ + 1` nodes touched | Θ(log n) — same nodes + call/return overhead per level |
| Range query | **O(log n)** — ≤ 2 combines per level ⇒ ≤ `2⌈log₂ n⌉` total | O(log n) — recursion touches ≤ 4 nodes per level ⇒ ≤ `4⌈log₂ n⌉` visits |
| Memory | **2n** values | 4n values (heap layout has gaps when n isn't a power of 2) + O(log n) stack |
| Constant factor | Small: tight loop, sequential-ish access, branch-friendly | 2–4× slower typically: call overhead, pointer chasing top-down |
| Code length | ~15 lines | ~40 lines |

Why recursive query is "≤ 4 nodes per level": at any depth, only the two
*border* nodes of the query range can be partially covered (everything between
them is fully covered and returns immediately). Each border node spawns at most
2 children ⇒ ≤ 4 active nodes per level. Same O(log n), bigger constant.

Both are asymptotically identical — **O(n) build, O(log n) query/update,
O(n) space**. The difference is constant factor, memory, code size, and
*what extensions each supports*.

---

## When each one wins

**Iterative wins (default in a 30-min interview):**

- Point update + range query — sum, min, max, gcd, xor: the standard ask
- Faster in practice (no call overhead, half the memory, better cache behavior)
- Much less code ⇒ fewer bugs under time pressure

**Recursive wins (know when to switch):**

1. **Lazy propagation (range update + range query)** — "add x to all of
   [l, r]". Lazy tags push top-down naturally; the recursive structure gives you
   the "visit parent before child" order for free. Iterative lazy exists but is
   genuinely tricky — in an interview, go recursive.
2. **Segment tree descent** — "find the first index with prefix sum ≥ x" or
   "first element > v in [l, r]" in O(log n) by walking down from the root,
   choosing a child by its stored value. Needs top-down traversal.
3. **Dynamic / implicit trees** — coordinates up to 1e18, create nodes on
   demand. Needs pointers/indices, i.e., recursion.
4. **Persistent segment trees** — k-th smallest in range, version history.
   Path-copying is top-down.

**Iterative caveat (the one real gotcha):** with the `2n` layout and `n` not a
power of two, the leaves are conceptually a *rotation* of the array, so ranges
map to nodes in a shuffled left/right order.

- Commutative ops (sum, min, max, gcd, xor): completely fine as-is — this is
  why the stress test uses n = 237.
- Non-commutative ops (matrix product, function composition): keep two
  accumulators — `resL = combine(resL, t[l++])`, `resR = combine(t[--r], resR)`,
  answer `combine(resL, resR)` — or pad n to a power of two.

---

## Worked example

`a = {5, 8, 6, 3, 2, 7, 2, 6}`, n = 8, sum tree, size-16 array:

```
index:  1                                      root = 39
        2  3                                   21, 18        (halves)
        4  5  6  7                             13, 8, 9, 9   (quarters)
        8..15                                  5, 8, 6, 3, 2, 7, 2, 6  (leaves)
```

`query(2, 6)` = sum of a[2..5] (half-open [2,6)):

```
l = 2+8 = 10,  r = 6+8 = 14
level 0: l=10 even -> skip;    r=14 even -> skip;    l=5, r=7
level 1: l=5 odd  -> res += t[5]=8, l=6;   r=7 odd -> res += t[6]=9, r=6
level 2: l=6 >= r=6 -> stop
res = 8 + 9 = 17? No — t[5] covers a[2..3]=6+3=9, t[6] covers a[4..5]=2+7=9
res = 9 + 9 = 18  ✓   (6+3+2+7 = 18, using just 2 nodes, not 4 leaves)
```

`update(3, 10)`: leaf 11 ← 10, then fix ancestors 5, 2, 1. Exactly
⌊log₂8⌋ + 1 = 4 nodes touched. New `query(2,6)` = 25. ✓

---

## Interview decision rule

> Point update only → **iterative** (write it in 3 minutes, spend the rest on
> the actual problem). Range update, tree descent, or persistence → **recursive**.

State the complexity out loud: "Build O(n), each op O(log n); iterative
version does at most 2⌈log₂n⌉ combines per query with a 2n-sized array."
