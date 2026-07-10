---
company: Apple
tags:
  - algorithm
  - backtracking
  - dynamic-programming
  - subset-sum
related: "[[README]]"
---

# Algorithm: All Subsets With Target Sum

## First principles — BFS across the solution space

Breadth-first over every approach an interviewer might expect you to mention:

1. **Brute force over all 2^n subsets** — for each bitmask, sum the selected values and
   keep the matches. O(2^n · n) always, even when only one subset matches. Correct,
   but does full work on inputs where almost nothing matches.
2. **Plain backtracking (DFS) with sort + break** — sort ascending; at each node try
   indexes `i ≥ start`, and `break` the moment `nums[i] > remaining` (non-negative,
   sorted ⇒ every later value fails too). Big practical win, but it can still wander
   deep into subtrees that have *some* small values left yet no exact completion.
3. **Classic subset-sum DP** — `dp[t] = can some subset make sum t`, O(n·target) via the
   0/1-knapsack bitset trick (O(n·target/64)). Answers *existence* only — it cannot list
   the subsets — but it's the seed of the winning idea.
4. **Backtracking + suffix-sum prune** — also abandon a branch when the total of the
   remaining suffix is less than `remaining`. Cheap and helpful, but still admits dead
   branches (suffix total large enough, exact sum still impossible, e.g. all-even suffix
   with odd remaining).
5. **Backtracking + suffix feasibility DP (chosen)** — precompute
   `reach[i][t] = "some subset of nums[i..n-1] sums to exactly t"`.
   The DFS asks `reach[i+1][remaining - nums[i]]` *before* taking element `i`.
   This is prune #4 made *exact*: no dead branch is ever entered.

Deeper (why you can't do better): the output itself can contain 2^Θ(n) subsets
(e.g. `[1,1,...,1]` with `target = n/2` has C(n, n/2) answers), so any correct algorithm
is Ω(total output size). "Optimal" therefore means **output-sensitive**: pay a polynomial
preprocessing cost, then pay only for what you emit. Approach 5 achieves exactly that.

## Chosen approach: sort + DFS + exact feasibility pruning

### Why it works

- **Sorting** gives two properties at once: emitted subsets come out (almost — see edge
  case below) in lexicographic order with no post-processing, and `nums[i] > remaining`
  lets us `break` instead of `continue` (everything after is at least as large).
- **`reach` table** is built right-to-left: `reach[n][0] = true`, and
  `reach[i][t] = reach[i+1][t] OR (t ≥ nums[i] AND reach[i+1][t − nums[i]])`
  — i.e. either skip element `i` or take it. This is the standard subset-sum recurrence,
  indexed by suffix so the DFS can query it at any depth.
- **Output-sensitivity proof sketch**: the DFS only enters a child if `reach` certifies
  at least one completion exists below it. So every root-to-node path in the explored tree
  extends to at least one emitted answer, and each answer has length ≤ n ⇒ the explored
  tree has at most K·n edges, where K = number of answers. Total: O(n·target + K·n).
- **Duplicates by index**: no "skip equal neighbors" dedup — the problem *wants*
  `[1,2],[1,2]` for `([1,2,2], 3)`. This is the opposite of LeetCode Subsets II; saying
  so explicitly is an easy interview point.

### The one edge case: duplicate zeros

DFS-by-index order equals value-lexicographic order *except* when duplicate zeros exist:
index order yields `[0], [0,0], [0]` but lexicographic order is `[0], [0], [0,0]`
(a prefix extension by zero keeps the sum, so a longer subset can be emitted before an
equal shorter sibling). A final `sort(ans)` on the near-sorted result is effectively
linear and restores the contract. Mentioning this unprompted is a strong signal;
most candidates assert "sorted input ⇒ sorted output" and are silently wrong on zeros.

### Why the alternatives lose

- **#1** pays 2^n even when K = 0. The feasibility check `reach[0][target]` answers K = 0
  in O(n·target) without exploring anything.
- **#2/#4** have no bound tying work to output size: adversarial inputs (many small values,
  unreachable exact remainder) make them explore exponentially many dead nodes.
- **#3** alone can't enumerate — but embedded per-suffix, it *is* the pruning oracle.

### Implementation decisions

- `vector<vector<char>>` for `reach` (not `vector<bool>`: bit-proxy overhead in the hot
  DFS path). If target is large, switch to one `std::bitset`/dynamic bitset per suffix
  and build with `b |= b << nums[i]` for an O(n·target/64) preprocessing.
- Record the answer when `remaining == 0` *then keep looping* — with zeros present,
  `[3,8]` and `[3,8,0]` are both valid answers.
- Emit `path` by copy at match time; path length ≤ n so copying is within the O(K·n) budget.
- Recursive lambda (`auto&& self`) keeps everything inside the class method — no globals,
  clean for a 30-minute interview.

### Complexity

- **Time**: O(n·target) preprocessing + O(K·n) enumeration + O(K log K · L) for the final
  near-sorted sort (effectively O(K·L)). K = number of answers, L = average answer length.
- **Space**: O(n·target) table + O(n) recursion + output.

### Where it breaks / follow-ups interviewers ask

- **Negative numbers**: sorting/break prune and DP indexing both assume non-negative.
  Fix: offset the DP by the sum of negatives, drop the `break` (keep `continue`).
- **Huge target, small n (≤ 40)**: O(n·target) DP is infeasible — switch to
  meet-in-the-middle: enumerate both halves, sort one, two-pointer/hash-join the pairs.
- **Count only, not enumerate**: pure DP, `ways[t] += ways[t - x]`, O(n·target).
- **Memory pressure**: the full per-suffix table is the price of exact pruning; the
  suffix-sum prune (#4) is the O(1)-memory fallback.
