---
company: Anthropic
tags:
  - algorithm
  - bitmask
  - backtracking
  - combinatorics
related: "[[README]]"
---

# Algorithm: Maximum-Length Unique-Character Subset

## First principles — BFS across the solution space

Before diving into the chosen technique, here's the space of approaches, breadth-first:

1. **Brute force over subsets of the original list** — try all 2^n subsets, concatenate,
   check for duplicate characters by scanning the string. O(2^n · total_length). Correct
   but does repeated, redundant character-scanning work.
2. **Precompute a per-word character signature, then brute-force subsets** — instead of
   rescanning strings, represent each word's character set as a 26-bit mask once. Checking
   two words for disjointness becomes a single AND instead of a nested character scan.
   This collapses the "check per subset" cost from O(total_length) to O(n).
3. **Subset enumeration over the masks** — loop `s` from `0` to `2^n - 1`, OR together the
   masks selected by `s`, compare popcount to the sum of lengths. O(2^n · n).
4. **Backtracking (DFS) over the masks with pruning** — same state space (2^n leaves in the
   worst case), but explored as a decision tree (skip / take at each word) so branches that
   overlap an already-used character are cut immediately, never reaching deeper recursion.
   This is strictly ≤ the work of plain enumeration and is the natural way to extend the
   idea if word count grows or further pruning (e.g., upper-bound estimates) is added later.

Deeper: since `len(words) ≤ 16`, the underlying state space is inescapably 2^16 in the worst
case (any two words might be disjoint, making every subset a candidate) — no polynomial
algorithm exists in general because this is equivalent to finding a maximum-weight set of
pairwise-disjoint sets (a form of set packing). The bound is small enough that exponential
enumeration is exactly the "optimal" answer expected in a 30-minute interview: don't over-engineer,
just make the exponential part as cheap as possible per node.

## Chosen approach: Bitmask encoding + branch-and-bound backtracking

### Step 1 — Filter and encode
For each word, build a 26-bit mask, one bit per letter it contains:

```
"un" → bit('u') | bit('n')
```

While building the mask, detect duplicates on the fly: if a bit is already set when we
try to add it again, the word has an internal repeat and is discarded immediately (it can
never appear in any valid concatenation).

### Step 2 — Order words to help pruning
Sort the filtered words by descending length (popcount of their mask). This has no effect
on correctness, but it means the DFS tends to find a strong candidate answer early, which
makes the upper-bound prune in Step 3 cut far more of the tree, far sooner. Also precompute
`pc[i]` (each word's length) once, up front, instead of recomputing `popcount` on every
recursive call.

### Step 3 — DFS with two prunes
Walk the ordered words with a recursive `dfs(i, usedMask, len)`, precomputing
`suffixSum[i] = pc[i] + pc[i+1] + ... + pc[n-1]` (an upper bound on how much length is
still obtainable from words `i..n-1`, assuming — best case — none of them overlap anything):

- **Bound prune:** if `i == n` OR `len + suffixSum[i] <= best`, update `best = max(best, len)`
  and return. The second condition is the branch-and-bound cut: even in the best possible
  case for the remaining words, this branch cannot beat what's already found, so there's no
  point recursing further. (This single check also subsumes the old base case, since
  `suffixSum[n] == 0`.)
- **Hard cap:** if `best == 26` already, return immediately — 26 is the maximum possible
  answer (only 26 letters exist), so nothing can improve on it.
- **Take branch:** if `usedMask & masks[i] == 0` (no shared character), recurse with
  `usedMask | masks[i]` and `len + pc[i]`. Tried *before* the skip branch, so promising
  paths are explored first.
- **Skip branch:** always recurse to `i + 1` unchanged.

### Why this prunes so much more than overlap-checking alone
Overlap-checking (the original version) only cuts a branch once it's *provably invalid*.
The upper-bound check cuts a branch as soon as it's *provably unable to win*, even if it's
still perfectly valid — e.g. once `best` is already 20 and only 3 letters worth of words
remain, there's no reason to enumerate any of their sub-combinations. Combined with
best-first ordering (Step 2), the "found a good answer early" effect means this triggers
very early in the search, often after only a handful of recursive calls.

---

## Complexity

| | Value |
|---|---|
| n (words, pre-filter) | ≤ 16 |
| Worst-case leaves (no pruning helps) | 2^16 = 65,536 |
| Work per node | O(1) (mask OR + array lookup) |
| **Absolute worst case** | **~10^5 ops** — microseconds regardless |

The worst case is unchanged (adversarial all-disjoint input still needs to confirm no
better combination exists), but realistic and adversarial-but-overlapping inputs — the
common case — terminate in a small fraction of that, since both prunes actively cut
live subtrees rather than only dead ones.

## Why not plain subset enumeration (the `for s in 0..2^n` loop)?

Equally valid and same worst-case asymptotic bound, but it always visits all 2^n subsets
and does O(n) work each, no matter what — it has no way to skip a subtree once it knows
that subtree can't win. The branch-and-bound DFS above strictly dominates it: same worst
case, faster in every other case.
