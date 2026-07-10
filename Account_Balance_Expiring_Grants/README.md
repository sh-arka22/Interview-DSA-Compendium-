---
company: Anthropic
difficulty: Medium
tags:
  - heap
  - priority-queue
  - lazy-deletion
  - greedy
  - simulation
---

# Account Balance with Expiring Grants

**Company:** Anthropic | **Difficulty:** Medium | **Role:** Software Engineer

## Problem

Model a prepaid-credit ledger. Credits arrive as time-limited grants, active during the
half-open interval `[timestamp, expire)`. Spending always draws from the grant that expires
soonest first (so the customer's longest-lived credit survives as long as possible), and any
unspent portion of a grant is forfeited the instant it expires.

Implement `solution(ops)` where `ops: List[List[str]]`, applied in non-decreasing timestamp
order, and return one string per `GET_BALANCE` call, in order.

- `["ADD_GRANT", timestamp, amount, expire]` — add `amount` credits, active on `[timestamp, expire)`.
- `["SPEND", timestamp, amount]` — drop expired grants, then deduct `amount`, earliest-expiry first. If the balance can't cover it, drain to zero and drop the rest.
- `["GET_BALANCE", timestamp]` — drop expired grants, return the remaining total.

## Examples

**Example 1:**
```
Input:  [["ADD_GRANT","0","100","10"],["ADD_GRANT","1","50","5"],
          ["GET_BALANCE","2"],["SPEND","3","60"],
          ["GET_BALANCE","4"],["GET_BALANCE","6"]]
Output: ["150","90","90"]
```
At t=2 both grants are active (100+50). SPEND 60 at t=3 drains the grant expiring at 5
(50) first, then takes 10 more from the grant expiring at 10, leaving 90.

**Example 2:**
```
Input:  [["ADD_GRANT","0","100","10"],["GET_BALANCE","10"]]
Output: ["0"]
```
The interval is half-open — at t=10 the grant has already expired.

## Approach

Min-heap ordered by expiry, with lazy deletion. See [ALGORITHM.md](ALGORITHM.md).

1. Push every grant onto a heap keyed by `expire`, and track a running `total` balance.
2. Before any `SPEND` or `GET_BALANCE`, pop grants whose `expire <= timestamp` off the heap,
   subtracting them from `total` (lazy expiration).
3. `SPEND` repeatedly takes from the heap's minimum (soonest-expiry) grant; if a grant is
   only partially consumed, push the remainder back (same expiry, smaller amount).
4. `GET_BALANCE` returns `total` as a string after the expiry sweep.

## Complexity

- Time: O(n log n) total across all `n` operations — each grant is pushed and popped from
  the heap at most twice (once on creation/partial-refill, once on removal), each O(log n).
- Space: O(n) for the heap, bounded by the number of `ADD_GRANT` calls.

## Constraints

- 1 ≤ len(ops) ≤ 10^5
- Timestamps are non-decreasing integers in [0, 10^15]
- amount is an integer in [1, 10^9]; expire > timestamp
- SPEND and GET_BALANCE must run efficiently even with many small grants
