---
company: Anthropic
tags:
  - algorithm
  - lru-cache
  - json-parsing
  - log-recovery
  - persistence
related: "[[README]]"
---

# Algorithm: Persistent LRU Cache Recovery and Append-Only Logging

## Overview

Three-phase pipeline:
1. **Recover** — replay the valid prefix of `existing_log` to restore LRU cache state.
2. **Process** — execute new operations, append one canonical log record per operation.
3. **Emit** — produce `final_cache` (sorted), `get_results`, and `updated_log`.

---

## Data Structures

| Structure | Purpose |
|---|---|
| `list<{key,val}>` front=MRU | O(1) splice for recency update and O(1) eviction from back |
| `unordered_map<key, list::iterator>` | O(1) lookup to find any key's node |
| `map<string,string>` (temp) | Canonical JSON: sorts object keys alphabetically at parse time |

---

## Key Design Decisions

### Canonical key normalization
Keys can be any JSON value including `{"b":2,"a":1}`. Two different orderings of the same object must map to the same cache slot. Solution: use the **canonical JSON string** (all object keys sorted recursively) as the map key. This is exactly Part 1's serializer, reused via `pVal()`.

### Malformed log recovery
`existing_log` may have a non-object final entry (the string `"MALFORMED"`). Stop replaying at the first non-object element; discard it and all subsequent entries. The valid prefix IS included in `updated_log`; the malformed tail is NOT.

### Log record format (canonical, keys sorted)
- put: `{"key":..., "op":"put", "value":...}` — k < o < v ✓
- get: `{"hit":true/false, "key":..., "op":"get"}` — h < k < o ✓

Every access (hit or miss) appends exactly one record.

### `final_cache` ordering
Sort entries by canonical key string (lexicographic). Collected by dumping `unordered_map` into a `std::map`.

---

## Complexity

- Recovery: O(L × K log K) — L = log length, K = max object keys per value
- Operations: O(N × 1) amortized — LRU get/put is O(1) via list+map
- Output build: O(C log C) — C = cache size for sorting final_cache
