---
company: Amazon
difficulty: Medium
tags:
  - design
  - linked-list
  - hash-map
  - lru-cache
---

# LRU Cache (Part 1)

**Company:** Amazon | **Difficulty:** Medium | **Role:** Software Engineer

## Problem

Design a fixed-capacity LRU cache of integer key-value pairs.

| Operation | Encoding | Returns |
|---|---|---|
| `get(key)` | `[0, key]` | value or -1 if missing |
| `put(key, val)` | `[1, key, val]` | nothing (evicts LRU if over capacity) |

Both operations make the key the most recently used. Only `get` results appear in the output vector.

## Examples

**Example 1** (capacity 2):
```
put(1,1), put(2,2), get(1)→1, put(3,3)[evicts 2], get(2)→-1,
put(4,4)[evicts 1], get(1)→-1, get(3)→3, get(4)→4
Output: [1, -1, -1, 3, 4]
```

**Example 2** (capacity 2):
```
put(1,1), put(2,2), put(1,10)[update→MRU], put(3,3)[evicts 2],
get(2)→-1, get(1)→10, get(3)→3
Output: [-1, 10, 3]
```

## Approach

Doubly linked list + hash map. See [ALGORITHM.md](ALGORITHM.md).

- List order: `head(dummy)↔MRU↔…↔LRU↔tail(dummy)`
- Map: `key → Node*` for O(1) lookup
- Every access/update → `remove` then `pushFront`
- Eviction → delete `tail->prev`
