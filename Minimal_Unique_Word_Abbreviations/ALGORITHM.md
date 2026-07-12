# Algorithm — Group by (length, lastChar), Sort, Adjacent LCP

## Step 1 — Prove conflicts are local to a group

An abbreviation is `P + d + c` where `P` = first `k` letters, `d` = decimal count of the
middle, `c` = last letter. Since `P` and `c` contain no digits, the string parses uniquely
back into `(P, |word|, c)`. Hence:

```
abbr(u, k1) == abbr(v, k2)   ⇒   |u| == |v|,  u.back == v.back,  k1 == k2,  u[0..k) == v[0..k)
```

So partition words into groups keyed by `(length, lastChar)`. Words in different groups can
never collide, at any `k`.

## Step 2 — Collapse the iteration into a closed form

Inside a group, word `u` collides at prefix length `k` with `v` iff `LCP(u, v) ≥ k`.
The statement's process keeps bumping `u` while *some* group member matches its prefix, so
it stops exactly at:

```
k(u) = 1 + max over v ≠ u in group of LCP(u, v)
```

(Well-defined: distinct words with equal length and last char have `LCP ≤ len − 2`, so
`k ≤ len − 1` — a legal prefix length.) No rounds, no rechecking.

## Step 3 — Sort to make max-LCP an adjacent property

For strings in sorted order, `LCP(a, c) ≤ min(LCP(a, b), LCP(b, c))` for `a ≤ b ≤ c` —
common prefixes can only shrink as you move away in sorted order. Therefore the maximum
LCP of `u` with the rest of its group is `max(LCP(prev, u), LCP(u, next))`. One pass of
adjacent LCPs (`L[1..m-1]`), then `k(sorted[i]) = 1 + max(L[i], L[i+1])`.

## Step 4 — The shorter-than check

Abbreviation length is `k + digits(len − k − 1) + 1`. Rather than reasoning about digit
counts, just build it and compare sizes — O(L) and immune to off-by-one traps
(`"dog" → "d1g"` is length 3 = len, so `"dog"` stays).

Reverted originals cannot re-introduce conflicts: originals contain no digits (never equal
an abbreviation) and inputs are distinct (never equal each other). So the check can be
applied independently per word, after `k` is fixed.

## Complexity

| Phase | Cost |
|---|---|
| Grouping | O(N) map ops |
| Sorting groups | O(N log N) comparisons × O(L) each |
| Adjacent LCPs + build | O(N·L) |
| **Total** | **O(N·L·log N)** time, O(N·L) space |

At N = L = 400 this is ~10⁶ character operations — instant. The naive simulation is
O(N² · L) per round with up to L rounds; fine here but the closed form is the answer that
distinguishes a candidate.

## Interview trap list

1. Assuming conflicts must be resolved iteratively — the closed form via max-LCP is the
   whole trick.
2. Comparing only pairwise within the *same current abbreviation* each round (the naive
   route) and forgetting a word can conflict with different words at different depths.
3. `word[k] + (len-k-1) + word.back()` with `char` arithmetic instead of `to_string` —
   breaks for middles ≥ 10 (`"i18n"`).
4. Applying the shorter-than check before resolving conflicts (order doesn't matter here,
   but only because originals can't collide — say why, don't assume).
