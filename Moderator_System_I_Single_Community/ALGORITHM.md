# Algorithm — Ordered Linked List + Rank-Key Comparison

## BFS pass: what is this problem, really?

Strip away the log-parsing and there are two things to maintain about "who is a moderator":

1. A **set** (membership — for `canRemoveMod`'s "is this person currently a mod" checks).
2. A **total order** over that set (for ranking + "who outranks whom").

Any sorted structure keyed on promotion timestamp gives you both. The only question is how
to keep it correct as moderators are added and removed, and how cheap each of the three
operations (`add`, `remove`, `compare-two-users`) needs to be.

## Naive idea and why it's one step short of optimal

Store `(user -> mostRecentAddedTimestamp)` in a hash map. `canRemoveMod` is then a plain
lookup + compare — O(1), fine. But `getModRanking` would need to **sort by timestamp on
every call** — O(k log k) per call, wasteful when it can be avoided entirely by keeping the
list *already* sorted.

## The structure: linked list in rank order + hash map

Keep a **doubly linked list** of `{user, key}` entries, ordered head (highest rank) to tail
(lowest rank), plus a hash map `user -> list iterator` for O(1) lookup and O(1) removal
(`std::list::erase` on an iterator doesn't shift anything else, unlike erasing from a
`vector`).

```
struct Entry { string user; long long key; };   // key: smaller = higher rank
list<Entry> order;                              // head..tail = highest..lowest rank
unordered_map<string, list<Entry>::iterator> pos;   // O(1) lookup
```

## The insight that removes timestamps entirely

Log lines arrive in **strictly increasing timestamp order** (given), and we process them in
that same order. So "the order logs are processed in" *is* "chronological order." Whenever a
user is (re-)added, their new promotion timestamp is larger than every currently-active
moderator's, simply because it's happening later in time. That means a fresh `"added"` event
always belongs at the **tail** of the ranking — no comparisons, no insertion sort, just
`push_back`.

So instead of storing the timestamp, each list node carries a **monotonically increasing
append counter** (`key`) — a stand-in for "how recently promoted." Appending with an
ever-growing counter reproduces the exact same order a timestamp comparison would, without
ever parsing one:

```cpp
if (action == "added") {
    order.push_back({target, counter++});
    pos[target] = prev(order.end());
} else {                              // "removed"
    order.erase(pos[target]);
    pos.erase(target);
}
```

## Answering queries

- **`getModRanking`**: walk the list head→tail collecting `user`. O(k), unavoidable since the
  whole list must be emitted.
- **`canRemoveMod(target, actor)`**: reject if `target == actor`; reject if either isn't in
  `pos`; otherwise compare `pos[actor]->key < pos[target]->key`. Smaller key means promoted
  earlier, which means higher rank. O(1) — this is exactly *why* an explicit key is kept
  instead of relying on raw list position (comparing positions directly would need an O(n)
  walk from one node to the other).

## Full trace — Example 1

```
logs: alice added(1), bob added(2), carol added(3), bob removed(4), bob added(5)

alice added -> order=[alice:0]                              pos={alice}
bob   added -> order=[alice:0, bob:1]                        pos={alice,bob}
carol added -> order=[alice:0, bob:1, carol:2]               pos={alice,bob,carol}
bob removed -> order=[alice:0, carol:2]                      pos={alice,carol}
bob   added -> order=[alice:0, carol:2, bob:3]                pos={alice,carol,bob}

getModRanking()            -> [alice, carol, bob]                      ✓
canRemoveMod(bob,alice)    -> key(alice)=0 < key(bob)=3        -> True ✓
canRemoveMod(carol,bob)    -> key(bob)=3 < key(carol)=2 ? no   -> False✓
canRemoveMod(alice,alice)  -> same user                        -> False✓
canRemoveMod(dave,alice)   -> dave not in pos                  -> False✓
```

## Complexity

- Construction: O(N) for N log lines. `canRemoveMod`: O(1). `getModRanking`: O(k).
- Space: O(k) for k currently active moderators.

## Where this goes next

- **Part II (multiple communities):** namespace this exact structure — one
  `{order, pos, counter}` triple per community, in an outer hash map. No new algorithmic
  idea, just scoping. See [[Moderator_System_II_Multiple_Communities]].
- **Part III (demotion):** once a moderator's rank can move independently of their promotion
  time, `key` can no longer be treated as immutable — it has to support an O(1) adjacent
  swap. See [[Moderator_System_III_Demotion]].
