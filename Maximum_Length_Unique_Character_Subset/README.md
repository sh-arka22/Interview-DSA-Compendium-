---
company: Anthropic
difficulty: Medium
tags:
  - bitmask
  - backtracking
  - subset-enumeration
  - combinatorics
---

# Maximum-Length Unique-Character Subset

**Company:** Anthropic | **Difficulty:** Medium | **Role:** Software Engineer

## Problem

Given a list of strings, choose a subset and concatenate the chosen strings. The concatenation
is valid only if every character in it is distinct (no letter repeats across the whole result).
Return the maximum possible length of a valid concatenation.

- A string that already has a repeated character internally can never be used.
- The empty selection is valid and has length 0.

## Examples

**Example 1:**
```
Input:  ["un", "iq", "ue"]
Output: 4
Notes:  "un"+"iq" = "uniq" (length 4). "un"+"ue" repeats 'u'.
```

**Example 2:**
```
Input:  ["cha", "r", "act", "ers"]
Output: 6
Notes:  "act"+"ers" uses a,c,t,e,r,s (length 6, all distinct).
```

**Example 3:**
```
Input:  ["aa", "bb"]
Output: 0
Notes:  Both words have an internal duplicate, so nothing can be chosen.
```

## Approach

Bitmask + branch-and-bound backtracking over the pre-filtered valid words. See [ALGORITHM.md](ALGORITHM.md).

1. Encode each word as a 26-bit character mask; discard words with an internal duplicate.
2. Sort remaining words longest-first, so strong candidates are found early.
3. DFS over the words: at each word, either take it (if its mask doesn't overlap the
   characters used so far) or skip it. Two prunes cut the search: a branch dies if it's
   invalid (overlap) or, more powerfully, if even the best possible completion of that
   branch (sum of all remaining word lengths) can't beat the current best. Search also
   stops outright once `best == 26`, since that's the max possible answer.
4. Track the best total length seen.

## Complexity

- Time: O(2^n) worst case (n ≤ 16 after filtering) — unavoidable, since this is a
  maximum-weight disjoint-set-packing problem. In practice, the branch-and-bound prune
  cuts far below this whenever a strong candidate is found early (real inputs and most
  adversarial ones), not just when a branch is provably invalid.
- Space: O(n) recursion depth + O(n) for the precomputed length/suffix-sum arrays.

## Constraints

- 1 ≤ len(words) ≤ 16
- 1 ≤ len(words[i]) ≤ 26
- Each word consists of lowercase English letters
