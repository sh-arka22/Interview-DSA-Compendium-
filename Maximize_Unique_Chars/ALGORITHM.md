---
company: Meta
tags:
  - algorithm
  - bitmask
  - subset-enumeration
  - combinatorics
related: "[[README]]"
---

# Algorithm: Maximize Unique Characters from a Word List

## Approach: Bitmask Subset Enumeration

Represent each valid word as a 26-bit integer and enumerate all 2^n subsets of valid words, checking character disjointness via bitwise OR.

---

## Steps

### 1. Filter and encode
For each word, build a bitmask: set bit `(c - 'a')` for each character `c`.

```
"aef" → bit0 | bit4 | bit5 = 0b110001 = 49
```

If `popcount(mask) != len(word)`, the word has an internal duplicate — **discard it** (it can never contribute to a valid concatenation).

### 2. Enumerate all 2^n subsets
For each subset bitmask `s` over the n valid words:
- OR all selected word-masks → `combined`
- Sum all selected word-lengths → `len`
- **Valid iff** `popcount(combined) == len` — no character appears in two different words
- Track maximum `len`

### 3. Why this check works
When two words share a character, the OR collapses that bit, making `popcount(combined) < len`. The check is a single comparison — no pairwise loop needed.

---

## Complexity

| | Value |
|---|---|
| n | ≤ 16 valid words |
| Subsets | 2^16 = 65,536 |
| Work per subset | O(n) = O(16) |
| **Total** | **~1M ops** |

Runs in microseconds. No faster general approach exists since all promising subsets must be examined.

---

## Why not backtracking?

Backtracking with pruning (skip word if mask overlaps) is also O(2^n) worst-case and slightly faster in practice due to early exits. The flat bitmask enumeration chosen here is simpler and equally fast at n ≤ 16.
