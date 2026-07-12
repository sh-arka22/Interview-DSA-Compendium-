---
source: LeetCode 2193
difficulty: Hard
time-budget: 40 min
tags:
  - hard
  - two-pointers
  - greedy
  - string
  - adjacent-swaps
  - inversions
algorithms:
  - Two Pointers (outermost-first pairing)
  - Greedy (rightmost-match exchange argument)
  - Inversion counting (BIT variant for follow-up)
---

# Minimum Number of Moves to Make Palindrome

**Source:** LeetCode 2193 | **Difficulty:** Hard | **Budget:** 40 min

---

## Problem Statement

Given a string `s`, return the **minimum number of moves** required to transform `s` into a **palindrome**. In one move you may swap any two **adjacent** characters.

**Note:** the input is guaranteed to be convertible into a palindrome.

**Constraints**

- `1 <= s.length <= 2000`
- `s` consists of lowercase English letters only
- `s` can always be converted into a palindrome in a finite number of moves

---

## Answer at a Glance

**Approach:** Two pointers `l, r` on the ends. Match `s[l]` with its **rightmost** occurrence `j` in `(l, r]` and bubble it to position `r` (cost `r - j`). If no match exists, `s[l]` is the unique odd-count character — swap it **one step inward** (cost 1) and retry. Shrink the window and repeat.

**Complexity:** `O(n²)` time, `O(n)` space (the working copy). For `n = 2000` that is ~4·10⁶ operations — instant. An `O(n log n)` BIT/inversion-counting version exists (see ALGORITHM.md §8) but is overkill for these constraints.

**Key facts**

1. Minimum adjacent swaps to reach a fixed target arrangement = number of **inversions** between current and target order.
2. In an optimal solution, **identical letters never cross each other** — crossing two equal letters is a wasted swap.
3. Fixing the **outer pair first** and pairing `s[l]` with its **rightmost** twin is never worse than any other pairing (exchange argument).

Full derivation, diagrams, worked examples and proof: [[ALGORITHM]].

## Files

- `ALGORITHM.md` — complete thought process from first principles, with diagrams
- `solution.cpp` — interview-optimal `O(n²)` greedy
- `solution_nlogn.cpp` — `O(n log n)` follow-up: same pairing, BIT inversion count
- `solution_segtree.cpp` — same `O(n log n)`, simplest iterative segment tree + classic recursive variant
- `solution_mergesort.cpp` — same `O(n log n)`, inversions counted by merge sort (no data structure)
- `test.cpp` — brute-force (BFS over swap states) cross-check + fixed cases
- `stack_approach_analysis.cpp` — why LIFO stack pairing fails (82.5% wrong on random tests); the correct "repaired" version is deque/FIFO pairing = the O(n log n) solution
