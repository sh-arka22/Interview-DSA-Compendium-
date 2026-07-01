---
company: Meta
difficulty: Medium
tags:
  - bitmask
  - subset-enumeration
  - combinatorics
  - greedy
---

# Maximize Unique Characters from a Word List (Part 2)

**Company:** Meta | **Difficulty:** Medium | **Role:** Software Engineer

## Problem

Given a list of lowercase words, choose any subset and concatenate them in any order such that the final string has **no repeated characters**. Return the maximum possible length.

- A word with internal duplicate characters cannot be used at all.
- Words in the chosen subset must have pairwise disjoint character sets.

## Examples

**Example 1:**
```
Input:  ["ab","cd","aef","gh"]
Output: 7
Notes:  "cd"+"aef"+"gh" = 7, all unique chars
```

**Example 2:**
```
Input:  []
Output: 0
```

**All disjoint:**
```
Input:  ["abc","def","ghi","jkl"]
Output: 12  (all four can be concatenated)
```

## Approach

Bitmask enumeration over all 2^n subsets of valid words. See [ALGORITHM.md](ALGORITHM.md).

1. Encode each valid word as a 26-bit mask (discard words with internal duplicates).
2. For each subset: OR all masks → valid if `popcount(OR) == sum of lengths`.
3. Return max valid length.

## Constraints

- 0 ≤ len(words) ≤ 16
- 1 ≤ len(words[i]) ≤ 26
