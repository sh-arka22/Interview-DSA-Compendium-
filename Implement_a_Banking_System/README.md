---
company: Anthropic
difficulty: Hard
tags:
  - simulation
  - design
  - stateful
  - hash-map
---

# Implement a Banking System

**Company:** Anthropic | **Difficulty:** Hard | **Role:** Software Engineer

## Problem

Design and implement an in-memory banking system. Given a list of operations in non-decreasing timestamp order, process them and return each result.

**`TRANSFER_EXPIRATION_MS = 86,400,000`** — a pending transfer created at `t` expires before any operation with timestamp `> t + EXPIRATION`. Expired transfers return their withheld amount to the source account.

### Operations

| Operation | Expire first? | Returns |
|---|---|---|
| `create_account(ts, id)` | No | True/False |
| `deposit(ts, id, amount)` | Yes | new balance or None |
| `top_outgoing(ts, n)` | Yes | top-n ids by completed outgoing (desc, lex tie-break) |
| `transfer(ts, src, tgt, amount)` | Yes | "transferN" or None |
| `accept_transfer(ts, id, transfer_id)` | Yes | True/False |
| `merge_accounts(ts, dest, src)` | Yes | True/False |

**transfer:** withholds amount from source immediately; not added to target or counted as outgoing until accepted.

**merge_accounts:** adds source balance + outgoing to dest; cancels transfers directly between the two accounts (withheld amount returns to dest); redirects all other pending transfers referencing source; deletes source.

## Examples

**Example 1** — basic operations:
```
Input:  [create_account alice, create_account alice, deposit bob 50, create_account bob,
         deposit alice 100, deposit bob 100, top_outgoing n=3, top_outgoing n=0]
Output: [True, False, None, True, 100, 100, ['alice', 'bob'], []]
```

**Example 2** — transfer lifecycle:
```
Input:  [create A, create B, deposit A 500, transfer A→B 200, top 2,
         accept_transfer A transfer1, accept_transfer B transfer1,
         top 2, deposit B 50, accept_transfer B transfer1]
Output: [True, True, 500, transfer1, ['A','B'], False, True, ['A','B'], 250, False]
```

## Approach

See [ALGORITHM.md](ALGORITHM.md).

**Key insight:** lazy expiration — scan active transfers before each eligible operation rather than maintaining a priority queue. Feasible at ≤ 2000 operations.
