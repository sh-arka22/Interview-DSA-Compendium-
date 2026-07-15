---
company: Amazon
tags:
  - algorithm
  - greedy
  - dynamic-programming
  - bounded-knapsack
related: "[[README]]"
---

# Algorithm: Minimum Bills and Coins to Make Change

This document follows a breadth-first survey of the whole solution space
first — every plausible approach, laid out side by side — before descending
into the depth of the one actually used in each part. Skim the survey to see
the landscape; read the deep dives to understand why the chosen path wins.

---

## Breadth-First Survey of Approaches

| Approach | Handles unlimited stock? | Handles limited stock? | Time complexity |
|---|---|---|---|
| **Brute-force recursion** (try every denomination at every step) | Correct but exponential | Correct but exponential | O(denominations^(amount/min_denom)) |
| **Greedy** (always take the largest fitting denomination) | ✅ Optimal for *this* denomination set | ❌ Can be suboptimal or wrongly infeasible | O(denominations) |
| **Unbounded knapsack DP** (classic coin-change DP, one coin at a time) | ✅ Optimal, but overkill | ❌ Doesn't respect stock limits at all | O(cents × denominations) |
| **0/1 knapsack, one item per physical coin** | Correct | ✅ Correct | O(cents × Σ inventory) — slow if stock is large |
| **Bounded knapsack via binary splitting** | Correct | ✅ Correct and fast | O(cents × Σ log(inventory)) |
| **Bounded knapsack via monotonic deque** | Correct | ✅ Correct, asymptotically best | O(cents × denominations) |

Part 1 sits on the **greedy** row: unlimited stock removes any need for DP
machinery at all, so reaching for it would be over-engineering.

Part 2 cannot use greedy (row 2's ❌) and the naive per-coin knapsack (row 4)
is correct but wasteful — with inventory counts in the thousands it degrades
badly. The implementation here sits on the **binary-splitting** row: a modest
[amortised](#glossary) constant-factor increase in code complexity buys an
exponential-to-logarithmic improvement in how many knapsack "items" each
denomination contributes.

---

## Part 1 — Greedy Coin Change (Deep Dive)

### The core loop

```
cents = round(amount * 100)
for denom in [2000, 1000, 500, 100, 25, 10, 5, 1]:
    take = cents // denom
    counts[denom] = take
    cents -= take * denom
    if cents == 0: break
```

### Why greedy is optimal here — the exchange argument

A denomination system is called **canonical** when greedy always produces
the minimum piece count. Not every set of denominations has this property —
`{1, 3, 4}` for a target of `6` is a classic counterexample (greedy gives
4+1+1 = 3 coins; optimal is 3+3 = 2 coins). The US-style set used here
(`1, 5, 10, 25, 100, 500, 1000, 2000` cents) **is** canonical. The intuitive
[exculpatory](#glossary) argument — the same one an interviewer wants to hear
out loud — is:

1. **Unlimited supply is essential.** Greedy's safety hinges on being able to
   swap any run of smaller coins for a bigger one whenever that run's value
   allows it. With finite stock, that swap might not be available — which is
   precisely why Part 2 needs a different tool.
2. **Each denomination is "reachable" from the one below it without waste.**
   For every denomination `d` in this set, the optimal way to represent any
   value strictly less than the next-larger denomination using denominations
   `≤ d` never needs more than one fewer unit of `d` than greedy would place —
   informally, no combination of smaller coins ever beats taking one more of
   the current-largest coin. This has been verified exhaustively for the
   standard US denomination set (a formal proof checks all remainders below
   each threshold — mechanical but not deep).
3. **Consequence:** since greedy never "overshoots" in a way a smarter choice
   could have avoided, it reaches the same total as any optimal decomposition,
   one denomination at a time, from the top down.

### The floating-point trap

`amount * 100` computed and compared in `double` is the single most common
way to fail this problem in an interview. `0.1` has no exact binary
[fixed-point](#glossary) representation, so an amount like `6.30` might
become `629.9999999...` or `630.0000000...1` depending on how it arrived.
`llround(amount * 100.0)` snaps to the nearest integer exactly once, at the
boundary — after that, every operation is on `long long` cents, which is
exact.

### Complexity

O(1) time, O(1) space — 8 fixed denominations, independent of `amount`.

---

## Part 2 — Bounded Knapsack via Binary Splitting (Deep Dive)

### Why greedy fails — a worked counterexample

`amount = $0.30`, `inventory = {1× $0.25, 3× $0.10}` (nothing else in stock).

- **Greedy:** take the $0.25 (largest that fits) → $0.05 remains → no nickel
  or penny in stock → **reports infeasible**.
- **Optimal:** three $0.10 coins → exactly $0.30 in 3 pieces.

Greedy's exchange argument silently assumed an infinite dime supply to "make
change" for whatever the large coin left behind. Finite stock invalidates
that assumption — greedy isn't just occasionally non-minimal here, it can be
outright *wrong about feasibility*.

### The DP formulation

Let `dp[j]` = minimum pieces to make exactly `j` cents using stock consumed
so far. `dp[0] = 0`; everything else starts at infinity (infeasible).

If a denomination had unlimited stock, this would be the standard unbounded
knapsack: iterate `j` upward, allowing reuse of the same denomination
arbitrarily many times. Here each denomination's use is capped by
`inventory[i]` — a **bounded** knapsack.

### Binary splitting — turning "bounded" into "0/1"

Treating each of `inventory[i]` individual coins as a separate 0/1-knapsack
item is correct but slow: `O(cents × Σ inventory)`. The fix borrows the same
trick used to represent any integer in binary — split a stock of `c` coins
into parcels of size `1, 2, 4, 8, ..., remainder`. Any quantity from `0` to
`c` can be assembled by choosing a subset of those parcels, exactly the way
any integer up to `c` is a sum of a subset of powers of two. This turns `c`
individual items into only `O(log c)` parcels, each folded into the knapsack
as a single 0/1 item worth `(parcel_size × denom)` cents and costing
`parcel_size` pieces.

```
for each denomination i:
    remaining = inventory[i]
    for parcel = 1, 2, 4, 8, ... while remaining > 0:
        take = min(parcel, remaining)
        remaining -= take
        value  = take * denom[i]
        pieces = take
        # standard 0/1 knapsack update, iterating cents DOWNWARD
        for j = cents downto value:
            dp[j] = min(dp[j], dp[j - value] + pieces)
```

**Why downward iteration matters:** iterating `j` from high to low ensures
each parcel is folded into `dp[]` at most once per pass. Iterating upward
would let the same parcel's value be reapplied to its own result within the
same pass, silently turning a bounded item into an unbounded one — a subtle
bug that produces answers that look plausible but overcount available stock.

### Correctness sketch

Every achievable stock usage for denomination `i` (any integer `0..inventory[i]`)
is expressible as a sum of a subset of that denomination's parcels, by the
same argument as binary representation of integers. Since the 0/1-knapsack
pass considers all subsets of items (implicitly, via the DP), and each
denomination's parcels are mutually exclusive-or-combinable exactly like
binary digits, the DP explores exactly the same feasible region as the naive
per-coin 0/1 knapsack — just with far fewer items.

### Complexity

- **Time:** `O(cents × Σᵢ log(inventory[i] + 1))` — each denomination
  contributes `O(log(inventory[i]))` parcels, each parcel costs `O(cents)`
  for its knapsack pass.
- **Space:** `O(cents)` — a single 1-D DP array, updated in place.

### Trade-offs and the further-optimized alternative

A **monotonic-deque bounded-knapsack** technique brings this down to
`O(cents × denominations)` — asymptotically better, since it removes the
`log(inventory)` factor entirely by processing each denomination's DP
"stripe" (indices congruent mod `denom`) with a sliding-window minimum. For
register amounts in the low thousands of cents and 8 denominations, binary
splitting is already comfortably fast (well under the deque version's extra
implementation complexity), which is the right trade-off to state out loud
in a 30-minute interview: correct and fast enough beats marginally faster
and harder to get right under time pressure.

---

## Complexity Summary

| Part | Approach | Time | Space |
|---|---|---|---|
| 1 | Greedy | O(1) | O(1) |
| 2 | Bounded knapsack, binary splitting | O(cents × Σ log(inventory)) | O(cents) |
| 2 (further optimization) | Bounded knapsack, monotonic deque | O(cents × denominations) | O(cents) |

---

## Glossary

*(Brief, advanced-vocabulary terms used above, with plain-English glosses.)*

- **Amortised** — averaged out over a sequence of operations, rather than
  measured on any single one in isolation.
- **Exculpatory** — used loosely here to mean "excusing/justifying" why
  greedy is safe in Part 1's unlimited-stock setting (its usual sense is
  legal: evidence that clears someone of blame).
- **Fixed-point** — a numeric representation with a set number of digits
  after the decimal point, as opposed to floating-point's sliding precision.
