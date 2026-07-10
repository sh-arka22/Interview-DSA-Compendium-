---
company: Apple
difficulty: Medium
tags:
  - backtracking
  - dynamic-programming
  - subset-sum
  - pruning
---

# All Subsets With Target Sum

**Company:** Apple | **Difficulty:** Medium | **Role:** Software Engineer

## Problem

Return all subsets whose values sum to `target`. Equal values at different indexes
are distinct choices (no deduplication). Return subsets sorted lexicographically
for deterministic output.

- Non-negative integers are assumed.
- Duplicate values at different indexes are distinct choices.

## Examples

**Example 1:**
```
Input:  ([2,3,7,8,10], 11)
Output: [[3,8]]
```

**Example 2:**
```
Input:  ([1,2,2], 3)
Output: [[1,2],[1,2]]
Notes:  Both 2s are distinct choices, so [1,2] appears twice.
```

## Approach

Sort + backtracking, with a suffix subset-sum feasibility DP (`reach[i][t]`) that
prunes every provably dead branch before entering it — the enumeration becomes
output-sensitive: O(n·target) preprocessing, then O(n) work per emitted answer.
See [ALGORITHM.md](ALGORITHM.md).

## Complexity

- **Time:** O(n·target) DP + O(K·n) enumeration, where K = number of matching subsets
  (K can be exponential — that lower-bounds ANY correct algorithm, since the output must be written).
- **Space:** O(n·target) for the DP table, O(n) recursion depth.
