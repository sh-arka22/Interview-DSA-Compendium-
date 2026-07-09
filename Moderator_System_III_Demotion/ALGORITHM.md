# Algorithm — O(1) Adjacent Swap via Splice + Key Exchange

## BFS pass: what breaks from Part II, and why

Parts I–II both leaned on one fact: rank == promotion order, and promotion order == the
order logs were processed in. `demote` breaks that fact on purpose — a user's rank can now
move for a reason that has nothing to do with when they were promoted. The moment that
happens, "derive the order from timestamps" stops being a valid strategy at all; the order
has to become a **first-class, directly-mutable thing**, and `canRemoveMod`'s O(1) key
comparison has to keep working *through* those mutations.

So the real question Part III asks is: can you support an **adjacent swap** in the
maintained order without paying O(n) to renumber everyone, and without breaking O(1) rank
comparison? Yes — because a swap of two neighbors only ever inverts the relative order of
that one pair. Nothing else in the list needs to change.

## Why not renumber everyone after a demotion?

The instinctive fix: after `demote`, reassign every node's key to its new list position
(`0, 1, 2, ...`) so keys stay contiguous. That's O(k) per demotion — wasteful, since only two
nodes actually changed relative order. The reframe: **only touch the two nodes that moved.**

## The two-part O(1) move

`demote(community, user)` finds `user`'s node `a` and its immediate successor `b` (the
person one rank below). Two things happen, both O(1):

```cpp
auto a = pos[user];
auto b = next(a);
if (b == order.end()) return;     // `a` already lowest -> no-op
swap(a->key, b->key);             // (1) only these two invert relative order
order.splice(a, order, b);        // (2) O(1): unlink b, relink it immediately before a
```

**(1) Key swap.** The key's only job is "smaller key sits earlier in the list." Before the
swap that invariant held for the whole list. The swap changes the relative order of exactly
one pair (`a`, `b`) — so exchanging only their two key values is the minimal edit that keeps
"key order == list order" true everywhere, with zero renumbering cascade.

**(2) Physical move via `splice`.** `list::splice(pos, list, it)` moves the single node `it`
to sit right before `pos`, in the same list, in O(1) — no allocation, and every *other*
iterator in the list stays valid (this is precisely why `std::list` over `std::vector` is the
right container once demote must exist: a `vector`-based order would need an O(k) shift on
every demotion). Given `..., a, b, ...`, splicing `b` to just before `a` yields `..., b, a,
...` — exactly "drop `a` by one slot."

## Why a later re-add still lands correctly

Even after arbitrary demotions have scrambled keys among existing members, a *newly*
(re-)added user still gets `counter++` — a value that has always been larger than any key
ever handed out. Swapping only exchanges two *already-issued* values; it never manufactures a
new maximum. So Part I's "append to tail" rule keeps working unmodified through any amount of
demotion.

## Full trace — Example 1

```
order (c1) = [user1:0, user2:1, user3:2]

demote(c1,user1):  a=user1(key0), b=user2(key1)
  swap keys  -> user1:1, user2:0
  splice user2 before user1 -> order=[user2:0, user1:1, user3:2]      ranking: [user2,user1,user3] ✓

demote(c1,user1):  a=user1(key1), b=user3(key2)
  swap keys -> user1:2, user3:1
  splice user3 before user1 -> order=[user2:0, user3:1, user1:2]      ranking: [user2,user3,user1] ✓
```

## Full trace — Example 2 (no-op cases)

```
order (c1) = [user1:0, user2:1, user3:2]

demote(c1,user1):  swap+splice -> order=[user2:0, user1:1, user3:2]           result: None
demote(c1,user3):  a=user3(key2), b=next(a)=end() -> no-op                    result: None
getModRanking(c1)  -> [user2, user1, user3]                                    ✓
demote(c1,ghost):  ghost not in pos -> no-op                                   result: None
getModRanking(c1)  -> [user2, user1, user3]  (unchanged)                       ✓
```

## Complexity

- `demote`: **O(1)** (one hash lookup, one key swap, one splice).
- `canRemoveMod`: **O(1)**, unchanged from Parts I–II — it never cared *why* the order is
  what it is, only that `key` comparison matches list order, which the swap preserves.
- `getModRanking`: **O(k)**. Construction: **O(N)**.
- Space: **O(total active moderators across all communities)**.

## Follow-ups

- **Promote-back-up / undo a demotion.** Symmetric operation: swap with `prev` instead of
  `next` — identical splice trick, mirrored.
- **Demote by k positions in one call.** Still O(k) in the worst case per call (k adjacent
  swaps) unless the spec allows an amortized/lazy representation; for a single-slot demote as
  specified here, O(1) is already optimal since you must touch at least the two nodes that
  changed order.
- **Persisting state across restarts.** Same append-only-log pattern as
  [[Persistent_LRU_Cache]] — replay `logs` to rebuild `order`/`pos`/`counter`, then replay any
  logged `demote` calls in order.
