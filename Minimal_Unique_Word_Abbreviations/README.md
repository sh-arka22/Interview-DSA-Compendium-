---
tags:
  - interview
  - string
  - hashing
  - sorting
  - lcp
---

# Minimal Unique Word Abbreviations

**Company:** Apple · **Difficulty:** Hard *(= LeetCode 527, Word Abbreviation)*

## Problem

Given `n` distinct lowercase words, abbreviate each as `first k chars + middleCount +
lastChar` starting at `k = 1`. While an abbreviation is shared by two or more words, bump
`k` for the conflicting words. If a word's abbreviation is not shorter than the word, keep
the word. Return the results in input order.

### Examples

| Input | Output | Why |
|---|---|---|
| `["dog","deer","deal"]` | `["dog","d2r","d2l"]` | `"d1g"` isn't shorter than `"dog"`. |
| `["abcdefz","abxdefz"]` | `["abc3z","abx3z"]` | Collide at `k=1,2`; unique at `k=3`. |

Constraints: `1 ≤ n ≤ 400`, `2 ≤ words[i].length ≤ 400`, all distinct.

## The insight — abbreviations are (prefix, length, lastChar) tuples

Letters are never digits, so `abc3z` parses **uniquely**: maximal letter prefix, digit run,
last char. Two abbreviations are equal iff the words share the **same length, same last
character, and the same first `k` letters**. Two consequences kill the simulation:

1. Conflicts only happen inside a **group** keyed by `(length, lastChar)`.
2. The iterative bumping converges to a closed form:
   `k(word) = 1 + max LCP with any other word in its group`.

And within a **sorted** group, the max LCP with anyone is achieved by an adjacent
neighbour — so `n−1` adjacent LCPs per group suffice. See `ALGORITHM.md`.

**Complexity**: O(N·L·log N) time, O(N·L) space — vs O(N²·L·rounds) naive simulation.

## Edge Cases

- **Short words** — length 2–3 can never be abbreviated shorter; always kept.
- **Max-depth conflict** — distinct words in one group have LCP ≤ len−2, so `k ≤ len−1`
  always yields a *valid* abbreviation, but it may fail the shorter-than check
  (`["aaaaab","aaaacb"]` → both originals).
- **Reverted originals never re-conflict** — a full word has no digits, so it can't equal
  any abbreviation; distinct inputs can't equal each other.
- **Uneven depths in one group** — `["internets","internals","intervals"]` → first two go
  to `k=7` (kept as originals), `intervals` stops at `k=6` → `"interv2s"`.
- **Multi-digit middles** — `"internationalization" → "i18n"`; use `to_string`, never a
  single char digit.

## Verification

`test.cpp` — 9 cases including the classic LC527 set. Additionally cross-checked against a
brute-force simulation of the literal statement process on 3000 random inputs
(3-letter alphabet to force heavy collisions): identical output.

## Run

```bash
g++ -std=c++17 -O2 -o test test.cpp && ./test
```
