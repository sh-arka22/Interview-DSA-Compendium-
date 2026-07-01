---
company: Anthropic
difficulty: Hard
tags:
  - lru-cache
  - persistence
  - json-parsing
  - log-recovery
---

# Persistent LRU Cache Recovery and Append-Only Logging (Part 2)

**Company:** Anthropic | **Difficulty:** Hard | **Role:** Software Engineer

## Problem

Simulate an LRU cache that persists state via an append-only log. Given a JSON input containing `capacity`, `existing_log`, and `operations`:

1. **Recover** the cache by replaying the valid prefix of `existing_log` (stop at any non-object entry — malformed tail).
2. **Process** each new operation (get/put), appending one canonical log record per access.
3. **Return** a JSON object with `final_cache`, `get_results`, and `updated_log`.

Keys and values are arbitrary JSON — keys may be objects/arrays and must be normalized (sorted object keys recursively) for consistent hashing.

## Examples

**Example 1** — recover then process:
```
capacity=2, existing_log: [put(a,1), put(b,2)]
operations: [get(a), put(c,3), get(b)]

Recovery:  cache = {b:2(MRU), a:1(LRU)}
get(a)  → hit=1,   cache=[a(MRU), b(LRU)]
put(c,3)→ evict b, cache=[c(MRU), a(LRU)]
get(b)  → miss=null

final_cache:  [["a",1],["c",3]]
get_results:  [1,null]
updated_log:  5 records (2 recovery + 3 new)
```

**Example 2** — malformed tail ignored:
```
existing_log: [put(x,1), "MALFORMED"]  → only put(x,1) replayed
operations:   [get(x)→1, put(z,3)]

final_cache:  [["x",1],["z",3]]
get_results:  [1]
updated_log:  3 records (1 recovery + 2 new)
```

## Approach

Recursive JSON parser (from Part 1) + `std::list` + `unordered_map`. See [ALGORITHM.md](ALGORITHM.md).

## Constraints

- 0 ≤ capacity ≤ 10^4
- 0 ≤ len(existing_log) + len(operations) ≤ 2×10^4
- All keys/values are JSON-serializable
- A non-object element in existing_log ends recovery immediately
