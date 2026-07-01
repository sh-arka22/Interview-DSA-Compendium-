---
company: Amazon
tags:
  - algorithm
  - linked-list
  - hash-map
  - lru-cache
related: "[[README]]"
---

# Algorithm: LRU Cache

## Data Structures

```
head (dummy MRU) <-> [most recent] <-> ... <-> [least recent] <-> tail (dummy LRU)

cache: unordered_map<key, Node*>
```

Two sentinel nodes (`head`, `tail`) eliminate all edge cases for insertion/removal — no null pointer checks ever needed.

## Core Operations (all O(1))

**remove(n):** Unlink by relinking prev and next around n.

**pushFront(n):** Splice n in immediately after `head` (MRU position).

**get(key):**
1. Miss → push -1.
2. Hit → remove node, pushFront (refresh recency), return val.

**put(key, val):**
1. Key exists → update val, remove, pushFront.
2. Key new → create node, insert into cache + pushFront. If `cache.size() > capacity`, evict `tail->prev` (LRU): remove it, erase from map, delete node.

## Complexity

- O(1) per get and put (hash map O(1) average + O(1) list splice)
- O(capacity) space
