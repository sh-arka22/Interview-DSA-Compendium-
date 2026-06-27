---
company: Google
tags:
  - algorithm
  - heap
  - two-pointer
  - google
related: "[[README]]"
---

# Algorithm: Rooms and Top-K Streams

---

## Problem 1 — Minimum Concurrent Meeting Rooms

### Approach 1: Two-Pointer on Sorted Events (Primary)

**Insight:** Sort starts and ends independently. Scan starts in order; for each start, check whether the _earliest-ending_ ongoing meeting has freed its room.

```
Sort starts[] and ends[] independently.
rooms = 0, maxRooms = 0, e = 0

for each start s (in sorted order):
    if s < ends[e]:          ← no room freed yet; need a new one
        rooms++
    else:                    ← ends[e] ≤ s: a room is freed and immediately reused
        e++                  ← advance the end pointer (consume one freed room slot)
    maxRooms = max(maxRooms, rooms)

return maxRooms
```

**Why `rooms` never decreases:**
`rooms` tracks cumulative room-slots allocated. When a room is freed and immediately reused (the `else` branch), the _net count_ stays the same: one slot freed = one slot taken. So `rooms` only grows; `maxRooms` captures the true peak.

**The critical boundary condition:**  
Use strict `<` in `s < ends[e]`, NOT `≤`. If `s == ends[e]`, the ending meeting frees its room at the exact moment the new one starts — the problem guarantees this should reuse the room (only 1 needed, not 2). Using `≤` in the wrong place here is the most common bug on this problem.

**Worked example:** `[[0,30],[5,10],[15,20]]`
```
starts = [0, 5, 15],  ends = [10, 20, 30]

i=0: s=0,  ends[0]=10.  0 < 10  → rooms=1. max=1.
i=1: s=5,  ends[0]=10.  5 < 10  → rooms=2. max=2.
i=2: s=15, ends[0]=10. 15 ≥ 10  → e=1.     max=2.  (rooms stays 2; slot reused)

Answer: 2  ✓
```

**Back-to-back test:** `[[0,5],[5,10]]`
```
starts = [0, 5],  ends = [5, 10]

i=0: s=0, ends[0]=5.  0 < 5   → rooms=1. max=1.
i=1: s=5, ends[0]=5.  5 ≥ 5   → e=1.     max=1.   ← only 1 room needed ✓
```

**Complexity:** O(N log N) time, O(N) space.

---

### Approach 2: Min-Heap on End Times (Alternative)

Sort meetings by start time. Maintain a min-heap of end times for ongoing meetings.

```
Sort intervals by start time.
minHeap = empty

for each [start, end] in sorted order:
    if heap not empty AND heap.top() ≤ start:
        heap.pop()           ← reuse the earliest-freed room
    heap.push(end)

return heap.size()           ← remaining heap entries = rooms in use at peak
```

The heap size at the end equals the minimum rooms needed because: every push that doesn't pop represents a room that couldn't be freed before the new meeting started.

**Complexity:** O(N log N) time, O(N) space.  
**vs. Two-Pointer:** Same complexity. Two-pointer avoids the heap constant factor and is slightly more elegant. The heap approach is more intuitive to many interviewers ("model the rooms explicitly").

---

### Sweep-Line Comparison

An alternative is to create +1 events for starts and −1 events for ends, sort all events by time (with tie-breaking: process end events BEFORE start events at the same time, since end-at-t frees for start-at-t), then scan to find the maximum prefix sum. Same O(N log N) complexity, same idea, slightly more code.

---

### Follow-up: Extremely Large Input (Doesn't Fit in Memory)

1. **External sort** the starts array and ends array (merge sort variant, processing chunk-by-chunk with merge passes).
2. **Stream** the two sorted sequences together using the same two-pointer logic — reading one element at a time, maintaining O(1) state (`rooms`, `maxRooms`, `e`).
3. **Total I/O:** O(N log N) with O(1) working memory beyond the sorted runs.

---

## Problem 2 — Top-K Stream

### Core Data Structure: Fixed-Size Min-Heap

Maintain a **min-heap of exactly `k` items**, where the _weakest_ item (most likely to be evicted) sits at the top for O(1) access.

**Weakness ordering** (weakest first = top of heap):
- Lower score is weaker.
- Among equal scores, **larger itemId is weaker** (because smaller id wins).

**Comparator:** `comp(a, b) = true` means `a` has LOWER priority (is STRONGER, sinks deeper).

```
comp((scoreA, idA), (scoreB, idB)):
    if scoreA ≠ scoreB: return scoreA > scoreB   // higher score = stronger = sinks
    return idA < idB                              // smaller id  = stronger = sinks
```

This makes the heap a "weakest-at-top" structure using C++'s default max-heap with custom comparator.

**add(itemId, score):**
```
if k == 0: return

if heap.size() < k:
    heap.push({score, itemId})
    return

// Is the new item strictly stronger than the current weakest in the top-k?
(topScore, topId) = heap.top()
stronger = (score > topScore) OR (score == topScore AND itemId < topId)

if stronger:
    heap.pop()
    heap.push({score, itemId})
// else: new item is too weak — discard
```

**Why "strictly stronger"?** If the new item is exactly as weak as the heap top (same score, same or larger id), we keep the existing item. This prevents phantom duplicates and correctly implements "smaller id wins."

---

### Tie-Breaking Deep Dive

Suppose k=2 and heap contains `{(score=10, id=3), (score=10, id=1)}`.
- Heap top = `(10, 3)` (larger id → weaker).
- New item `(id=5, score=10)`: is 5 < 3? **No** → not stronger → rejected. ✓
- New item `(id=2, score=10)`: is 2 < 3? **Yes** → stronger → pop `(10,3)`, push `(10,2)`. ✓
  - Top-2 becomes `{(10,1), (10,2)}`. ✓

---

### getTopK

The heap doesn't maintain sorted output order. To return items sorted by score DESC / id ASC:

```
copy heap into a vector  → O(k) copy
sort by (score DESC, id ASC)  → O(k log k)
repack as (itemId, score) pairs
```

Total: O(k log k). Since k is typically small, this is acceptable.

---

### Complexity

| Operation | Time | Notes |
|---|---|---|
| `add` | O(log k) | Heap push/pop |
| `getTopK` | O(k log k) | Copy + sort; call infrequently if hot |
| Space | O(k) | Only the top-k window is stored |

**match() call count:** N/A (no oracle). Total match comparisons: O(N log k) for N items added.

---

### Follow-up: Dynamic k

If k can grow, the discarded items are gone. Solutions:
- **Store all N items** in an ordered set / BST (e.g., `std::set` or order-statistics tree).
  - `add`: O(log N); `getTopK(k)`: O(k) with an iterator.
  - Space: O(N).

---

### Follow-up: Delete an Item

Heaps don't support O(log N) arbitrary deletion by identity.

**Lazy deletion:**
```
deleted = unordered_set<int>()   // itemIds marked for deletion
id_to_score = unordered_map<int,int>()

delete(itemId):
    deleted.insert(itemId)

add(itemId, score):
    id_to_score[itemId] = score
    // proceed as before, but...

getTopK():
    while heap.top().id is in deleted:
        heap.pop()       // flush stale entries on demand
    // return next k valid items
```

Amortized O(log N) per deletion; each item is pushed and popped at most once.

---

### Follow-up: Score Update

Equivalent to `delete(itemId)` followed by `add(itemId, newScore)`. Requires lazy deletion support above, plus a `id_to_score` map to verify that a heap entry hasn't been superseded by a newer score (double-check on pop).

---

### Why a Heap is Natural — and Its Limits

**Natural because:** We need repeated access to the MINIMUM of the top-k set (to decide eviction), and heap gives O(1) peek + O(log k) update. This is exactly the min-heap use case.

**Limits:**
- No O(log k) arbitrary deletion by identity.
- No efficient key-change (update score): requires delete + re-insert, and deletion is hard (see above).
- `getTopK` requires O(k log k) sort (not O(k)) unless we maintain a separate sorted structure.
- Not persistent — if you need historical snapshots, you'd need a different structure.
