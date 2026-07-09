# Algorithm — Namespacing Part I's Structure by Community

## BFS pass: what actually changes from Part I?

Read the spec closely and the answer is: **nothing algorithmic.** Part I's invariant —
"a linked list in rank order, plus a hash map for O(1) lookup, with an append-counter
standing in for the promotion timestamp" — holds per community *unchanged*. The only new
requirement is that operations must not leak across community boundaries. So the fix is
pure namespacing: take the one `{order, pos, counter}` triple from Part I and give every
community its own copy.

```cpp
struct Community {
    list<Entry> order;                                  // Part I's `order`, unchanged
    unordered_map<string, list<Entry>::iterator> pos;    // Part I's `pos`, unchanged
    long long counter = 0;                               // Part I's `counter`, unchanged
};
unordered_map<string, Community> communities;             // NEW: one triple per community
```

## Why this is the right (and sufficient) generalization

Two independent things justify this being "just" namespacing rather than a new structure:

1. **Ranking is defined only within a community** ("rankings are computed within a community
   only") — so there is never a need to compare a key from one community against a key from
   another. Each community's counter can start fresh at 0 with no cross-talk.
2. **Timestamps are globally increasing across communities, but only relatively meaningful
   within one** — a community's local append-counter reproduces "promoted earlier within
   this community" exactly as well as the global timestamp would, for the exact same reason
   argued in Part I (processing order == chronological order, so a fresh add is always the
   newest in *that* community).

## What every operation costs now

Every method gains exactly one hash lookup — `communities.find(community)` (or
`communities[community]` on the write path, which default-constructs an empty `Community` on
first use) — before falling into the identical Part I logic:

```cpp
// add
Community& c = communities[community];
c.order.push_back({target, c.counter++});
c.pos[target] = prev(c.order.end());

// canRemoveMod
auto ci = communities.find(community);
if (ci == communities.end()) return false;          // unseen community -> default
... same key comparison as Part I, scoped to ci->second.pos ...

// getModRanking
auto ci = communities.find(community);
if (ci == communities.end()) return {};              // unseen community -> empty
... same head->tail walk as Part I, over ci->second.order ...
```

That one extra lookup is the entire diff from Part I — everything else (append-to-tail,
O(1) erase, O(1) key comparison) is verbatim.

## Full trace — Example 1

```
logs: c1:alice added(1), c1:bob added(2), c2:bob added(3),
      c1:carol added(4), c1:bob removed(5), c1:bob added(6)

c1:alice added -> c1.order=[alice:0]
c1:bob   added -> c1.order=[alice:0, bob:1]
c2:bob   added -> c2.order=[bob:0]                          (independent counter, independent list)
c1:carol added -> c1.order=[alice:0, bob:1, carol:2]
c1:bob removed -> c1.order=[alice:0, carol:2]
c1:bob   added -> c1.order=[alice:0, carol:2, bob:3]

getModRanking(c1)            -> [alice, carol, bob]                         ✓
getModRanking(c2)            -> [bob]                                       ✓
canRemoveMod(c1,bob,alice)   -> key(alice)=0 < key(bob)=3        -> True    ✓
canRemoveMod(c1,carol,bob)   -> key(bob)=3 < key(carol)=2 ? no   -> False   ✓
canRemoveMod(c2,bob,alice)   -> alice not in c2.pos              -> False   ✓
```

## Complexity

- Construction: O(N) — one extra hash lookup per line vs. Part I.
- `canRemoveMod` / `getModRanking`: identical to Part I, plus O(1) for the community lookup.
- Space: O(total active moderators across all communities).

## Where this goes next

Part III adds `demote`, which breaks the assumption that rank == promotion order. That
forces `key` to become mutable within a community — see
[[Moderator_System_III_Demotion]] for the O(1) adjacent-swap technique. The per-community
namespacing built here carries over completely unchanged.
