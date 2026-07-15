---
company: Unspecified
tags:
  - algorithm
  - hash-map
  - ordered-set
  - ranking
related: "[[README]]"
---

# Algorithm: Employee Time Tracking and Ranking

Breadth-first survey of the approach space first, then a deep dive into the
one implemented.

---

## Breadth-First Survey of Approaches

| Approach | ADD/CLOCK_IN/CLOCK_OUT | TOP_TIME (k) | Notes |
|---|---|---|---|
| **Hash map + resort on every `TOP_TIME`** | O(1) avg | O(n log n) per query | Simplest to write; degrades badly if `TOP_TIME` is called often relative to `n` employees. |
| **Hash map + max-heap of all employees** | O(1) avg | O(k log n) to pop k, but heap doesn't support O(log n) *update* of an existing key without a lazy-deletion scheme | Standard heaps are awkward here because `CLOCK_OUT` changes an existing employee's key — heaps don't support efficient decrease/increase-key without extra bookkeeping (indexed heap). |
| **Hash map + ordered set (balanced BST), key = `(-totalTime, id)`** | O(1) avg for map; O(log n) for the ranking update on `CLOCK_OUT` | O(k) — a prefix walk, no sort | Chosen approach. Balanced-BST ordered sets support O(log n) erase-and-reinsert on key change, which is exactly what a `CLOCK_OUT`-driven ranking update needs. |
| **Fenwick tree / order-statistics tree over compressed time values** | O(log n) | O(k log n) | Overkill here — useful when you need arbitrary rank queries ("what's the Nth highest"), which isn't asked for; adds coordinate-compression complexity for no payoff. |

The ordered-set approach wins because the *update pattern* is the deciding
factor: `CLOCK_OUT` changes one employee's rank key, and `TOP_TIME` just
wants a prefix of the sorted order. A balanced BST ordered by the right key
gives both in the complexity a 30-minute interview implementation should
target — without needing a hand-rolled indexed heap.

---

## Deep Dive — Hash Map + Ordered Set

### Employee record (hash map, keyed by id)

```cpp
struct Employee {
    string name;
    long long wage;
    bool clockedIn;
    long long clockInTime;
    long long totalTime;       // sum of completed shifts only
    int completedShifts;       // used as the "has this employee ever
                                // completed a shift" membership check
};
unordered_map<string, Employee> employees_;
```

O(1) average lookup/insert for `ADD`, `CLOCK_IN`, `CLOCK_OUT`, `TOTAL_TIME`.

### The ranking key trick

```cpp
set<pair<long long, string>> ranking_;   // key = (-totalTime, id)
```

`std::set` (and `std::pair`) compare **ascending** by default. Encoding the
key as `(-totalTime, id)` turns that ascending order into exactly the output
order `TOP_TIME` needs, for free:

- First component ascending on `-totalTime` ⟺ `totalTime` **descending**
  (highest worked time first).
- Second component ascending on `id` ⟺ alphabetical tie-break for equal
  totals.

No comparator lambda, no post-query sort — the set's natural iteration order
*is* the ranking.

### Why erase-before-mutate is required

A `set`'s ordering invariant is built from the *key value at insertion time*.
If an employee's `totalTime` changes (a second completed shift) without
first removing the old key, the set now silently holds a stale entry at the
old position AND you'd be inserting a second entry for the same employee at
the new position — corrupting both the ordering invariant and creating a
duplicate.

```cpp
if (e.completedShifts > 0) ranking_.erase({-e.totalTime, id});  // OLD key
e.totalTime += duration;
e.completedShifts += 1;
ranking_.insert({-e.totalTime, id});                             // NEW key
```

The `completedShifts > 0` guard skips the erase on an employee's *first*
completed shift — they were never inserted, so there's no stale key to
remove. This is also exactly what implements Assumption 3 from
`README.md`: an employee is only ever present in `ranking_` once they've
completed at least one shift, so `TOP_TIME` naturally excludes anyone who
hasn't.

### CLOCK_IN / CLOCK_OUT validity

Both are pure state-machine transitions on a 2-state machine per employee
(`clockedIn ∈ {false, true}`):

```
CLOCK_IN:  requires clockedIn == false  → sets clockedIn = true,  records clockInTime
CLOCK_OUT: requires clockedIn == true   → sets clockedIn = false, folds duration into totalTime
```

Any call attempted from the wrong state is rejected (`return false`)
*before* touching any member variable — this is what makes "invalid
transitions do not mutate state" (Example 2) hold by construction rather
than by an extra guard clause bolted on afterward.

### TOP_TIME query

```cpp
for (const auto& [negTime, id] : ranking_) {
    if (out.size() >= k) break;
    out.push_back({id, -negTime});
}
```

Because `ranking_` is already in the exact required order, this is a
straight prefix walk: `O(k)` once you have `begin()`, which is `O(1)` for a
balanced BST. No sorting step exists anywhere in the query path.

---

## Complexity Summary

| Operation | Time | Notes |
|---|---|---|
| `ADD` | O(1) avg | hash map insert |
| `CLOCK_IN` | O(1) avg | hash map lookup + field writes |
| `CLOCK_OUT` | O(log n) | hash map lookup O(1) avg + ordered-set erase/insert O(log n) |
| `TOTAL_TIME` | O(1) avg | hash map lookup |
| `TOP_TIME(k)` | O(k) | prefix walk over an already-sorted structure |
| **Overall, m operations, n employees** | **O(m log n)** worst case | dominated by `CLOCK_OUT` calls |
| Space | O(n) | one hash map entry + at most one ordered-set entry per employee |

Compare against the naive "resort every `TOP_TIME` call" approach: with `q`
`TOP_TIME` calls interleaved among `n` employees, that's `O(q · n log n)`
total — the ordered-set approach amortizes the sorting cost across updates
instead of repeating it on every read, which matters a great deal once `q`
and `n` both grow.

---

## Design Extensions (Follow-up Discussion)

### True point-in-time queries (arbitrary historical `ts`)
The current design assumes `ts` is always "now" in log order (Assumption 1).
If a query needed to ask "what was `TOTAL_TIME` for `a` as of some earlier
timestamp, possibly out of log order," the flat `totalTime` field isn't
enough — you'd need either:
- A per-employee list of `(completion_ts, cumulative_total)` checkpoints,
  binary-searched at query time (`O(log shifts)` per query, `O(shifts)` space
  per employee), or
- A persistent/versioned ordered-set (path-copying balanced BST) for
  `TOP_TIME`, since the ranking itself changes over time and a single live
  `ranking_` can't answer "what was the ranking at time T" after the fact.

### Payroll extension (using the currently-unused `wage` field)
A natural Part 2 would add something like `TOTAL_PAY(ts, id)` = `totalTime *
wage`, or a `TOP_PAY(ts, k)` ranking mirroring `TOP_TIME`'s structure with a
second ordered set keyed by `(-totalPay, id)`. Wage is already stored on
`Employee` specifically to make this a drop-in extension rather than a
schema change.

### Deleting/deactivating an employee
Not supported today. A tombstone flag (skip in `TOP_TIME` and reject further
`CLOCK_IN`/`CLOCK_OUT`) is the lowest-disruption option; a hard delete would
also need to erase the employee's `ranking_` entry if present.

### k changes dynamically across calls
Already supported as-is — `k` is just a parameter to `topTime()`/`TOP_TIME`
per call, not fixed at construction time (contrast with a fixed-size top-k
stream, which is a different problem shape).
