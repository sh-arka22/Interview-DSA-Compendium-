---
tags:
  - interview
  - hash-map
  - transactions
  - undo-log
  - system-design
---

# In-Memory Key-Value Database with Nested Transactions

**Company:** Anthropic · **Difficulty:** Medium

## Problem

Implement an in-memory key-value store (string keys, string values) driven by a list of
commands. It must support basic reads/writes **plus nested transactions** (a `BEGIN` inside an
open transaction opens a child). Return one output line per command that produces a result.

| Command | Behaviour |
|---|---|
| `["SET", key, value]` | Associate `value` with `key`. No output. |
| `["GET", key]` | Current value of `key`, or `"NULL"` if unset. |
| `["DELETE", key]` | Remove `key`. No output. Deleting a missing key is a **no-op**. |
| `["COUNT", value]` | How many keys currently map to `value` (as a string). Must be efficient. |
| `["BEGIN"]` | Open a (possibly nested) transaction. No output. |
| `["COMMIT"]` | Permanently apply all changes since the **outermost** `BEGIN`, closing **every** open transaction. `"NO TRANSACTION"` if none open. |
| `["ROLLBACK"]` | Discard changes of the **innermost** open transaction and close only it. `"NO TRANSACTION"` if none open. |

**Semantics:** reads see the innermost transaction's view layered over its parents and the
committed base. A write inside a transaction is visible to reads in that transaction and its
descendants, but must not touch committed state until `COMMIT`.

### Examples

| Input | Output | Why |
|---|---|---|
| `[[SET,a,10],[GET,a],[BEGIN],[SET,a,20],[BEGIN],[DELETE,a],[GET,a],[ROLLBACK],[GET,a],[COMMIT],[GET,a]]` | `[10, NULL, 20, 20]` | `GET a`=10; after nested `DELETE` →`NULL`; inner `ROLLBACK` restores `a=20`; `COMMIT` flattens `a=20`; final `GET a`=20. |
| `[[COMMIT]]` | `[NO TRANSACTION]` | Nothing open. |

## Core idea — one live map + per-transaction undo logs

The naive design keeps a **stack of maps** (one per transaction) and walks it on every read —
making `GET` O(depth) and `COUNT` O(N). Instead:

- Keep **one `live` map** that always holds the current visible state. Apply every write
  immediately.
- Keep an **undo log per open transaction** (`frames`, a stack). Before a write, journal the
  key's *previous* state into the **innermost** frame.
- `ROLLBACK` replays the innermost frame **in reverse** to restore; `COMMIT` just discards all
  frames (the writes are already in `live`).
- Maintain `valueCount` (value → #keys) **incrementally** on every write, so `COUNT` is O(1).

This is exactly write-ahead **undo logging**, the way real databases implement rollback.

```
frames = [ frame0 , frame1 , frame2 ]     depth = 3
           outer    middle   innermost  <- writes journal here; ROLLBACK undoes here
```

## Edge Cases

- **`COMMIT` / `ROLLBACK` with no open transaction** → `"NO TRANSACTION"` (guard on empty
  `frames`).
- **`DELETE` of a missing key** → true no-op: nothing applied *and nothing journaled*, so a
  later `ROLLBACK` won't "resurrect" a phantom key.
- **Same key written twice in one transaction** → the frame holds multiple undo records for it;
  reverse-order replay lands on the *original* value, not the intermediate one.
- **`COMMIT` flattens every level at once**, not just the innermost — all frames dropped.
- **`ROLLBACK` closes only the innermost** — the enclosing transaction's writes survive and a
  key is restored to *its* value, not the committed base.
- **Writes outside any transaction are permanent** — `recordUndo` no-ops when `frames` is
  empty, so nothing is journaled.
- **`COUNT` correctness across moves** — when a key's value changes, the old value's count is
  decremented (and its entry erased at 0) so stale values don't report phantom counts.
- **`GET` on an unset key** → the literal string `"NULL"`.

## Complexity

- `SET` / `DELETE`: **O(1)** amortized (one journal push + one map write).
- `GET` / `COUNT`: **O(1)**.
- `BEGIN`: **O(1)**. `ROLLBACK`: O(records in innermost frame). `COMMIT`: O(total journaled
  records) to clear.
- Overall **O(total commands)** work; space O(distinct keys + journaled writes).
