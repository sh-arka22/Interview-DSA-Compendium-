---
tags:
  - interview
  - stack
  - parsing
---

# Compute Exclusive CPU Time from Nested Logs

**Company:** Decagon · **Difficulty:** Medium

## Problem

A single-threaded CPU runs `n` functions with ids `0 … n-1`. Each log entry is a string
`"functionId:start:timestamp"` or `"functionId:end:timestamp"`:

- A **start** log means the function begins executing at the **start** of that timestamp.
- An **end** log means the function finishes at the **end** of that timestamp — so
  **end timestamps are inclusive** (the function occupies that whole unit).
- Calls are **properly nested like a stack**; the CPU runs at most one function at a time.

Return `ans` where `ans[i]` is the **exclusive** execution time of function `i` — time spent
running its own code, **excluding** time spent inside functions it calls.

### Examples

| Input | Output | Why |
|---|---|---|
| `n=2, ["0:start:0","1:start:2","1:end:5","0:end:6"]` | `[3, 4]` | fn 0 runs at 0,1,6 → 3; fn 1 runs at 2–5 → 4 |
| `n=1, ["0:start:0","0:end:0"]` | `[1]` | start and end at 0; inclusive end → 1 unit |

## Constraints

- `1 <= n <= 10^5`
- `1 <= logs.length <= 10^5`
- Logs are consistent: every `start` has a matching `end`, ends are non-decreasing in time,
  and the first log is a `start`.

## Note on the input format

The logs are **colon-delimited strings**, not JSON, so the repo's
[[_templates|JSON parser template]] does **not** apply here. Parsing is a two-split on `:`
into `(id, type, timestamp)`.

## Edge Cases

- **Single-unit function** — `start` and `end` on the same timestamp → counts as `1`
  (inclusive end). This is the classic off-by-one trap; the `+1` on `end` handles it.
- **Recursion** — the same id can sit on the stack more than once
  (e.g. `["0:start:0","0:start:2","0:end:5","0:end:6"]` → `[7]`). Both frames accumulate
  into the same `ans[id]`, which is exactly what "exclusive time of function i" wants.
- **Back-to-back (non-nested) calls** — when a function ends, the stack becomes empty before
  the next `start`; the guard `if (!stk.empty())` avoids crediting a nonexistent caller.
- **Deep nesting** — a parent gets credit only for the gaps *before* a child starts and
  *after* it returns, never for the child's span.
- **Large timestamps** — an exclusive time fits in `int` for the given range; the stack depth
  is bounded by the number of `start` logs.

## Complexity

- **Time:** `O(L)` over the log entries (each pushed/popped once). `O(1)` per parse.
- **Space:** `O(D)` for the stack, where `D` is the max nesting depth (`≤ L`), plus `O(n)` output.
