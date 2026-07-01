---
company: Anthropic
difficulty: Medium
tags:
  - json
  - parsing
  - canonicalization
  - cache-key
---

# Canonical Cache Key Generation (Part 1)

**Company:** Anthropic | **Difficulty:** Medium | **Role:** Software Engineer

## Problem

Given a JSON string `{"func_name": "f", "args": [...], "kwargs": {...}}`, produce a canonical cache-key string so that:

- Positional arguments (`args`) keep their original order.
- Keyword arguments (`kwargs`) are sorted by key alphabetically.
- Nested JSON objects anywhere in the input also have their keys sorted.

## Output Format

```
["func_name", canonical_args, canonical_kwargs]
```

## Examples

**Example 1** — no arguments:
```
Input:  {"func_name":"f","args":[],"kwargs":{}}
Output: ["f",[],{}]
```

**Example 2** — kwargs reordered:
```
Input:  {"func_name":"f","args":[],"kwargs":{"b":2,"a":1}}
Output: ["f",[],{"a":1,"b":2}]
```

**Nested object (args):**
```
Input:  {"func_name":"h","args":[{"y":2,"x":1}],"kwargs":{}}
Output: ["h",[{"x":1,"y":2}],{}]
```

## Approach

Recursive-descent JSON parser. Arrays preserve order. Objects are inserted into `std::map` (sorted by key) at every level. See [ALGORITHM.md](ALGORITHM.md).

## Constraints

- Input is valid JSON.
- Values are any JSON-serializable types (null, bool, number, string, array, object).
- 1 ≤ len(func_name) ≤ 200
- Total input ≤ 10^5 characters.
