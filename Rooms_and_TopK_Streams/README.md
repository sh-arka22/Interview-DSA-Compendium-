---
company: Google
difficulty: Medium
round: Technical Screen
tags:
  - google
  - medium
  - heap
  - two-pointer
  - sorting
algorithms:
  - Two-Pointer on Sorted Events
  - Min-Heap on End Times
  - Fixed-Size Min-Heap with Custom Comparator
---

# Rooms and Top-K Streams

**Company:** Google | **Role:** Software Engineer | **Difficulty:** Medium | **Round:** Technical Screen

---

## Problem 1: Minimum Concurrent Meeting Rooms

Given a list of meeting time intervals `intervals`, where each interval is `[start, end)` and `start < end`, return the minimum number of rooms required so all meetings can be held.

**Rules:**
- If `intervals` is empty, return `0`.
- A meeting ending at time `t` **frees its room** for another meeting starting at time `t`.
- Do not assume the time range is small — the algorithm must be O(N log N).

**Example:**
```
Input:  [[0, 30], [5, 10], [15, 20]]
Output: 2

Timeline:
  t=0:  [0,30] starts                    → 1 room
  t=5:  [5,10] starts, [0,30] ongoing    → 2 rooms
  t=10: [5,10] ends
  t=15: [15,20] starts (reuses freed room) → 2 rooms
  t=20: [15,20] ends
  t=30: [0,30] ends                      → 0 rooms
```

---

## Problem 2: Maintain the Top-K Items in a Stream

You receive a stream of `(itemId, score)` pairs. For a fixed integer `k`, maintain the top-`k` items with the **largest scores** after processing each new item.

**Interface:**
```
add(itemId, score) — process a new item and update the maintained top-k set
getTopK()          — return the current top-k items
```

**Rules:**
- If fewer than `k` items have been seen, return all seen items.
- **Tie-breaking:** among equal scores, prefer **smaller itemId**.

**Example:**
```
k = 2
add(3, 10)  → top-2: {(3,10)}
add(1, 10)  → top-2: {(1,10),(3,10)}   ← same score, smaller id=1 wins
add(2,  5)  → top-2: {(1,10),(3,10)}   ← score=5 not strong enough
```

---

## Edge Cases

### Problem 1

| # | Case | Why It Matters |
|---|---|---|
| 1 | **Empty input** | Return 0 immediately. |
| 2 | **Single meeting** | Return 1, trivially. |
| 3 | **Back-to-back: `[0,5],[5,10]`** | End-at-5 frees the room; start-at-5 reuses it → **1 room**, not 2. The strict `<` in the overlap check is critical here. |
| 4 | **Chain of N back-to-back meetings** | All sequentially reuse the same room → 1 room. |
| 5 | **All meetings same interval** | Every meeting overlaps → N rooms. |
| 6 | **No overlaps at all** | 1 room; each meeting ends before the next starts. |
| 7 | **Fully nested: `[1,10],[2,8],[3,6]`** | All three overlap simultaneously → 3 rooms. |
| 8 | **Two freed rooms, one new meeting** | Meeting reuses only ONE freed room; the other freed slot stays in the pool. Algorithm handles this correctly (e advances once per new start that can reuse a room). |
| 9 | **Duplicate intervals** | `[[2,5],[2,5],[2,5]]` → 3 rooms. |
| 10 | **Negative time values** | Must work since times can be any integer. |
| 11 | **Large time values (e.g., `[0, 10^9]`)** | Algorithm uses sorting, not bucket counting — safe for any integer range. |
| 12 | **Meetings touching at negative boundary** | `[[-10,-5],[-5,0]]` → 1 room (same end-frees-for-start rule). |
| 13 | **Out-of-memory follow-up** | External merge sort on starts/ends files; then stream both sorted sequences with two pointers. O(N log N) I/O, O(1) memory beyond the sorted chunks. |

### Problem 2

| # | Case | Why It Matters |
|---|---|---|
| 1 | **k = 0** | Always return empty. Guard against this before any heap operations. |
| 2 | **Fewer than k items seen** | Return all seen items — heap never reaches capacity. |
| 3 | **All same score** | Tie-breaking by smallest itemId must be correct. Top-k = k items with smallest ids. |
| 4 | **Exact duplicate `(itemId, score)`** | New item is NOT strictly stronger than existing one → discarded. No phantom duplicates in top-k. |
| 5 | **Score exactly matches heap top** | Only admitted if itemId is strictly smaller. `≤` comparison is wrong here — use strict `<`. |
| 6 | **Negative scores** | Comparison by score still works (−3 > −10 → −3 is stronger). No special-casing needed. |
| 7 | **k = 1** | Degenerates to "track the best item seen so far." Must still respect tie-breaking (smallest id on tie). |
| 8 | **k > stream length** | Return all N seen items sorted by score DESC, id ASC. |
| 9 | **Score update (follow-up)** | Basic version doesn't support it. Need a hash map `id → score` + lazy deletion to handle re-insertion. |
| 10 | **Delete item (follow-up)** | Heaps don't support O(log N) arbitrary deletion. Use lazy deletion: mark id as deleted in a set; skip marked items on pop. |
| 11 | **Dynamic k (follow-up)** | If k increases, we no longer have the discarded items. Must store all N items (e.g., sorted set or BST). |
| 12 | **Integer overflow in score** | Use `long long` if scores can be large; comparisons remain safe. |

---

## Clarifying Questions

**Problem 1:**
1. Are intervals guaranteed to have `start < end`? (Yes, per problem.)
2. Can time values be negative or zero? (Algorithm handles both.)
3. Half-open `[start, end)` vs. closed `[start, end]` — matters for boundary. (Given: half-open / end-exclusive.)

**Problem 2:**
1. Are itemIds guaranteed to be unique in the stream, or can the same id appear multiple times?
2. Is there a maximum value for scores (overflow concern)?
3. Does `getTopK()` need to return items in a specific sorted order?
4. Can `k` change between calls (affects data structure choice)?

---

## Follow-up Complexity Summary

| Variant | add | getTopK | Space |
|---|---|---|---|
| Basic (min-heap size k) | O(log k) | O(k log k) | O(k) |
| Dynamic k (sorted set, all N items) | O(log N) | O(k) | O(N) |
| With delete (lazy deletion) | O(log N) amortized | O(k + deletions) | O(N) |
| With score update | O(log N) | O(k) | O(N) |

---

## Algorithm

See [ALGORITHM.md](./ALGORITHM.md) for full details.

**Algorithms Used:**
- **Problem 1:** Two-Pointer on Sorted Events · Min-Heap on End Times · Sweep Line
- **Problem 2:** Fixed-Size Min-Heap with Custom Weakness Comparator · Lazy Deletion (follow-up)
