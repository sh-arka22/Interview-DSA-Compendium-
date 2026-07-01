---
company: Anthropic
tags:
  - algorithm
  - json-parsing
  - recursive-descent
  - canonicalization
related: "[[README]]"
---

# Algorithm: Canonical Cache Key Generation

## Approach: Recursive-Descent JSON Parser with Sorted Object Keys

Parse the input JSON once and re-serialize it with object keys sorted at every nesting level. The canonical string is the cache key.

---

## Core Idea

Two serialization rules:
1. **Arrays** — preserve element order exactly.
2. **Objects** — sort keys alphabetically before serializing (recursively).

A `std::map<string,string>` enforces sorted key order automatically at every object level.

---

## Parser Structure

```
pVal()  — dispatch by first character
  '"'  →  pStr()   copy verbatim, preserve \escape sequences
  '['  →  pArr()   recurse per element, preserve order
  '{'  →  pObj()   recurse per value, collect into std::map (auto-sorted)
  'n'  →  "null"
  't'  →  "true"
  'f'  →  "false"
  else →  number   copy until delimiter
```

`pObj()` is where sorting happens:
1. Parse each `"key": value` pair.
2. Insert into `map<string,string>` — this sorts keys.
3. Serialize in map iteration order (alphabetical).
4. Values are themselves the canonical form (already recursed).

---

## Top-Level Handling

The input object `{func_name, args, kwargs}` is parsed without `pObj()` so the three fields can be extracted by name regardless of their order. Output: `[fn, canonical_args, canonical_kwargs]`.

---

## Complexity

- O(N log K) time — N = input size, K = max keys in any single object (log K for map insertions)
- O(N) space — for the output string and map storage
- O(D) stack depth — D = nesting depth of the JSON
