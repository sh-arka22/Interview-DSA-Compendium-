---
company: Amazon
difficulty: Easy (Part 1) / Medium (Part 2)
round: Software Engineer
tags:
  - amazon
  - greedy
  - dynamic-programming
  - knapsack
  - currency
algorithms:
  - Greedy Coin Change (canonical denomination system)
  - Bounded Knapsack via Binary Splitting
---

# Minimum Bills and Coins to Make Change

**Company:** Amazon | **Role:** Software Engineer | **Difficulty:** Easy → Medium | **Round:** N/A

---

## Problem Statement

You are building the change-dispensing logic for a self-checkout cash register.

**Denominations** (bills and coins, US-style):

| Bills | Coins |
|---|---|
| $20, $10, $5, $1 | $0.25, $0.10, $0.05, $0.01 |

### Part 1 (Easy) — Unlimited Stock

Given a purchase change `amount` in dollars (e.g. `6.35`), compute the minimum
total number of physical pieces (bills + coins) needed to make up that exact
amount, and report the per-denomination breakdown. Stock is unlimited, so a
**greedy** strategy — always take as many of the largest denomination that
still fits — is provably optimal for this denomination set (see
[ALGORITHM.md](./ALGORITHM.md) for why).

**Output:** a list of 9 integers —
`[total, c2000, c1000, c500, c100, c25, c10, c5, c1]` — where `total` is the
piece count and the remaining 8 entries are counts for `$20, $10, $5, $1,
$0.25, $0.10, $0.05, $0.01` respectively (cents-descending order).

### Part 2 (Medium) — Limited Inventory

The register's drawer now holds a **finite stock** of each denomination.
Given `amount` and an `inventory` array (8 non-negative integers, same
cents-descending order), return the minimum total pieces to make exact change
**without exceeding stock**. If exact change is impossible, return `-1`.

Finite stock breaks the exchange argument that makes greedy optimal in Part
1, so Part 2 requires dynamic programming (**bounded knapsack**) instead.

---

## Examples

### Part 1

```
Input:  amount = 6.35
Output: [4, 0, 0, 1, 1, 1, 1, 0, 0]
        4 pieces: $5 + $1 + $0.25 + $0.10 = $6.35
```

```
Input:  amount = 0
Output: [0, 0, 0, 0, 0, 0, 0, 0, 0]
```

### Part 2

```
Input:  amount = 0.30, inventory = [0,0,0,0,1,3,0,0]   (one $0.25, three $0.10)
Output: 3   (three $0.10 coins — greedy would take the $0.25 and get stuck)
```

```
Input:  amount = 6.35, inventory = [10,10,10,10,10,10,10,10]
Output: 4   (ample stock reproduces the unlimited-greedy answer)
```

```
Input:  amount = 0, inventory = [0,0,0,0,0,0,0,0]
Output: 0   (zero change needs zero pieces, regardless of drawer contents)
```

---

## Edge Cases

| # | Case | Why It Matters |
|---|---|---|
| 1 | **Floating-point dollars** | `amount * 100` in `double` can drift off an exact cent value (0.1 is a repeating binary fraction). Convert to integer cents with `round()` **once**, immediately, and do everything else in integers. |
| 2 | **`amount = 0`** | Both parts must return `0` pieces without touching the loop/DP body incorrectly. |
| 3 | **Exact single-denomination amounts** | e.g. `$20.00` should use exactly one $20 bill — no residual smaller denominations. |
| 4 | **Greedy trap (Part 2 only)** | Limited stock can force a genuinely worse — or infeasible — greedy path even though a better/valid combination exists. See the $0.30 example above. |
| 5 | **Infeasible inventory (Part 2 only)** | Must return `-1` cleanly, not a negative/garbage piece count or a crash. |
| 6 | **Exact stock, zero slack (Part 2 only)** | If inventory exactly covers the amount with one specific combination, the DP must find it — not report `-1` because it "used up" stock on a wrong-but-valid partial path. |
| 7 | **Large per-denomination stock (Part 2 only)** | Stock counts in the thousands must not make the DP construction itself slow — this is exactly what binary splitting avoids. |

---

## Constraints

**Part 1**
- `amount >= 0`, at most two decimal places (a whole number of cents).
- `amount` fits comfortably in a 64-bit integer once converted to cents.
- Always convert dollars to integer cents first (`round(amount * 100)`).

**Part 2**
- `amount >= 0`, at most two decimal places; convert to integer cents.
- `inventory` has exactly 8 non-negative integers, in the order
  `[$20, $10, $5, $1, $0.25, $0.10, $0.05, $0.01]`.
- Return `-1` when no combination within stock makes exact change.
- Greedy is **not** correct under limited stock; a DP (bounded knapsack) is required.

---

## Algorithm

See [ALGORITHM.md](./ALGORITHM.md) for the full breadth-first survey of
approaches followed by a deep dive into the chosen solution for each part.

**Algorithms Used:** Greedy Coin Change (Part 1) · Bounded Knapsack via
Binary Splitting (Part 2)
