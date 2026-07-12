---
tags:
  - interview
  - matrix
  - string
  - sorting
  - diagonal-traversal
---

# Sort Bottom-Left To Top-Right Diagonals

**Company:** Capital One · **Difficulty:** Medium

## Problem

Given a char matrix, form the string along every ↗ diagonal (bottom-left to top-right) and
return the **diagonal indexes** sorted by those strings, ties broken by smaller index.

### Examples

| Input | Output |
|---|---|
| `[[c,a],[b,d]]` | `[0,2,1]` |
| `[[a]]` | `[0]` |

## The real problem: reverse-engineering the index convention

The statement never defines which diagonal is index 0 — Example 1 is the spec. The ↗
diagonals of `[[c,a],[b,d]]` are `"c"`, `"ba"`, `"d"`. Testing conventions:

| Convention | diag 0,1,2 | Sorted | Match? |
|---|---|---|---|
| `d = r+c` (textbook, top-left first) | `c`, `ba`, `d` | `[1,0,2]` | ✗ |
| `d = r+c` reversed (bottom-right first) | `d`, `ba`, `c` | `[1,2,0]` | ✗ |
| **Bottom row L→R, then up left column** | `ba`, `d`, `c` | `[0,2,1]` | ✓ |

So: starting cells are enumerated **from the bottom-left corner along the bottom row
left→right, then up the left column**; each string is read walking up-right (`r--, c++`)
from its start:

```
i < cols : start (rows-1, i)
i >= cols: start (rows+cols-2-i, 0)
```

## Algorithm

Build the `rows+cols-1` strings (each cell belongs to exactly one diagonal → O(R·C) total),
`iota` the indices, `stable_sort` by string — stability *is* the tie-break-by-index rule,
no comparator clause needed.

**Complexity**: O(R·C) build + O(D log D) string comparisons, D = R+C−1. O(R·C) space.

## Edge Cases

- **Ties** — equal strings (e.g. all-`'a'` grid) keep index order; `stable_sort` handles it.
- **Prefix ordering** — `"a" < "ab"`: shorter prefix sorts first (std::string `<` does this).
- **1×N** — every diagonal is a single bottom-row cell; index = column.
- **N×1** — diag 0 is the *bottom* cell, then upward: `[d],[b],[c]` → `b(1) < c(0) < d(2)`.

## Run

```bash
g++ -std=c++17 -O2 -o test test.cpp && ./test
```
