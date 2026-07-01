---
company: Anthropic
tags:
  - algorithm
  - simulation
  - stateful
related: "[[README]]"
---

# Algorithm: Implement a Banking System

## Approach: Stateful Simulation with Lazy Expiration

**State:**
- `accounts` — hash map from account_id to `{balance, completedOutgoing}`
- `transfers` — vector of `{source, target, amount, createdAt, active}`
- Transfer counter implicit in `transfers.size()`

**Expiration (lazy):** Before each operation that requires it, scan all active transfers. Any with `createdAt + 86_400_000 < timestamp` is deactivated and its withheld amount returned to the current source account.

---

### Operations

| Operation | Expire first? | Returns |
|---|---|---|
| `create_account` | No | "True"/"False" |
| `deposit` | Yes | balance string or "None" |
| `top_outgoing` | Yes | Python list string |
| `transfer` | Yes | "transferN" or "None" |
| `accept_transfer` | Yes | "True"/"False" |
| `merge_accounts` | Yes | "True"/"False" |

**transfer:** Withhold amount from source balance immediately. The pending amount is neither available for further transfers nor counts as completed outgoing until accepted.

**accept_transfer:** Only the current target can accept. Adds amount to target balance and to source's `completedOutgoing`. Idempotent rejection on inactive transfers.

**merge_accounts (dest ← src):**
1. Add `src.balance` and `src.outgoing` to `dest`.
2. Scan pending transfers:
   - Transfer directly between `dest` and `src` (either direction) → cancel; withheld amount returns to `dest` (the surviving account).
   - Transfer sourced from `src` → redirect source to `dest`.
   - Transfer targeting `src` → redirect target to `dest`.
3. Delete `src`.

---

## Output Encoding

The platform compares C++ strings to Python `str()` output:
- Boolean → `"True"` / `"False"`
- None → `"None"`
- Integer → `to_string(n)`
- List → `"['a', 'b']"` (Python single-quote list repr)

---

## Complexity

- O(T) per operation for expiration scan (T = total transfers created, ≤ 2000)
- O(A log A) for `top_outgoing` sort (A = number of accounts)
- **Overall: O(ops × T)** — acceptable for ops ≤ 2000
