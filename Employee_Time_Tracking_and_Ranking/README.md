---
company: Unspecified
difficulty: Medium
round: N/A
tags:
  - design
  - hash-map
  - ordered-set
  - ranking
algorithms:
  - Hash Map State Store
  - Ordered Set Ranking (Balanced BST key encoding)
---

# Employee Time Tracking and Ranking

**Company:** Unspecified | **Role:** Software Engineer | **Difficulty:** Medium | **Round:** N/A

> **Reconstructed spec.** Only two worked examples were provided, not a full
> problem statement. Everything below the two Examples is a design decision
> made to explain the observed input/output pairs consistently — see
> **Assumptions** for exactly what was inferred and why. If you have the
> original prompt, compare it against these assumptions before relying on
> this as an exact match.

---

## Problem Statement (Reconstructed)

Design an in-memory system that tracks employee clock-in/clock-out events and
answers ranking queries over total worked time. Operations are supplied as a
list of string tokens, each beginning with an operation name and a timestamp:

| Operation | Args | Returns |
|---|---|---|
| `ADD` | `ts, id, name, wage` | `bool` — `false` (no mutation) if `id` already exists |
| `CLOCK_IN` | `ts, id` | `bool` — `false` (no mutation) if `id` is unknown or already clocked in |
| `CLOCK_OUT` | `ts, id` | `bool` — `false` (no mutation) if `id` is unknown or not currently clocked in |
| `TOTAL_TIME` | `ts, id` | `int` sum of completed-shift durations, or `None` if `id` is unknown |
| `TOP_TIME` | `ts, k` | list of `[id, totalTime]`, top `k` by total time descending, alphabetical tie-break, only employees with at least one completed shift |

---

## Examples

### Example 1

```
Input:
  [["ADD", 0, "a", "dev", 10],
   ["CLOCK_IN", 1, "a"],
   ["CLOCK_OUT", 6, "a"],
   ["TOTAL_TIME", 6, "a"],
   ["TOP_TIME", 6, 1]]

Output:
  [True, True, True, 5, [["a", 5]]]
```

A completed shift (clock-in at t=1, clock-out at t=6) contributes 5 units to
both `TOTAL_TIME` and the `TOP_TIME` ranking.

### Example 2

```
Input:
  [["ADD", 0, "a", "dev", 10],
   ["ADD", 1, "a", "qa", 9],
   ["CLOCK_OUT", 2, "a"],
   ["CLOCK_IN", 2, "missing"],
   ["TOTAL_TIME", 2, "missing"]]

Output:
  [True, False, False, False, None]
```

Duplicate `ADD` on an existing id, `CLOCK_OUT` on someone never clocked in,
and any operation on an unknown id all fail cleanly — `False`/`None` — and
leave existing state untouched.

---

## Assumptions

Because the source material stopped at two examples, the following design
decisions were made explicit rather than guessed silently. Each is
independently testable and called out in `test.cpp`:

1. **Operations arrive in non-decreasing timestamp order** (a forward log,
   the same convention used elsewhere in this repo, e.g. the in-memory
   logging system). `TOP_TIME`/`TOTAL_TIME`'s `ts` argument reflects "state as
   of this point in the log", not a historical point-in-time query against
   past state. A true point-in-time query (arbitrary `ts` in the past) would
   need a persistent/versioned structure — see **Design Extensions** in
   `ALGORITHM.md`.
2. **Only completed shifts count.** An employee currently clocked in
   contributes nothing to `TOTAL_TIME` or `TOP_TIME` until they clock out.
   This directly matches the stated note: *"a completed shift contributes to
   total time and ranking."*
3. **An employee with zero completed shifts is excluded from `TOP_TIME`
   entirely** — not listed with a time of `0`. Read literally, the note says
   a completed shift is what grants ranking membership, not just a
   value to sum.
4. **`TOP_TIME` ties break alphabetically by id, ascending.** Unspecified by
   the examples (no ties appear in either); chosen as the most common
   convention for this class of problem.
5. **Wage is stored but unused.** Nothing in either example reads it back;
   it's kept on the employee record for forward compatibility with a
   possible payroll extension, but no current operation computes pay.

---

## Edge Cases

| # | Case | Why It Matters |
|---|---|---|
| 1 | **Duplicate `ADD`** | Must return `false` and leave the original name/wage/state completely untouched. |
| 2 | **`CLOCK_IN`/`CLOCK_OUT`/`TOTAL_TIME` on an unknown id** | Returns `false`/`None` without creating a phantom record. |
| 3 | **Double `CLOCK_IN`** (already clocked in) | Rejected; critically, must **not** overwrite the original `clockInTime` — a later `CLOCK_OUT` must still measure from the *first* clock-in. |
| 4 | **Double `CLOCK_OUT`** (not currently clocked in) | Rejected; must not double-count or corrupt the accumulated total. |
| 5 | **Multiple completed shifts for one employee** | Durations accumulate (`TOTAL_TIME` sums all completed shifts), and the employee appears exactly once in `TOP_TIME` with the summed value — not once per shift. |
| 6 | **Zero-duration shift** (clock-in and clock-out at the same timestamp) | Still counts as *completed* (eligible for ranking) with a total contribution of `0` — completion is about the transition happening, not the duration being positive. |
| 7 | **`TOP_TIME` with `k` exceeding eligible employees** | Returns however many are eligible; does not pad or error. |
| 8 | **`TOP_TIME` with `k = 0`** | Returns an empty list. |
| 9 | **`TOP_TIME` before any shift has completed** | Returns an empty list, even if employees have been `ADD`ed or are currently clocked in. |
| 10 | **Ranking key updates on repeat `CLOCK_OUT`** | The employee's position in the ranking structure must be *replaced*, not duplicated, when their total time changes across multiple shifts. |

---

## Constraints

- `0 ≤` number of operations `≤ 100,000` (typical interview-scale bound).
- Employee ids and names are non-empty strings; wage is a non-negative integer.
- Timestamps are non-negative integers, non-decreasing across the operation
  log (see Assumption 1).
- `k ≥ 0` for `TOP_TIME`.

---

## Algorithm

See [ALGORITHM.md](./ALGORITHM.md) for the full breadth-first survey of
approaches, the ordered-set key-encoding trick, and complexity analysis.

**Algorithms Used:** Hash Map state store · Ordered Set (balanced BST) for
O(log n) ranking updates and O(k) top-k queries, with no per-query sort.
