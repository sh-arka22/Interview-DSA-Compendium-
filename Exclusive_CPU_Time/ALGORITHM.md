# Algorithm — Exclusive CPU Time (Stack)

## Core idea

Because calls are properly nested and single-threaded, the set of "currently running"
functions behaves exactly like a **call stack**. The function on top of the stack is the one
actually holding the CPU. We walk the logs once, maintaining:

- `stk` — ids of functions currently executing (top = running).
- `prev` — the timestamp at which the current top last **(re)started** owning the CPU.

Whenever control changes hands, we settle the elapsed time onto whoever was running.

## Transitions

For each log `(id, type, ts)`:

**start:**
```
if stack not empty:  ans[top] += ts - prev     // top ran over [prev, ts-1]
push id
prev = ts
```
The child begins *at the start of* `ts`, so the parent's last run ended at `ts-1` →
`ts - prev` units (no `+1`).

**end:**
```
ans[top] += ts - prev + 1                       // end is inclusive → owns ts too
pop
prev = ts + 1                                    // caller resumes at the next unit
```

## Why the `+1` asymmetry

- A `start` at `ts` is a *boundary*: the previous owner stops **before** `ts`, so its interval
  is `[prev, ts-1]`, length `ts - prev`.
- An `end` at `ts` is **inclusive**: the function owns `ts` fully, so its interval is
  `[prev, ts]`, length `ts - prev + 1`, and the parent can't resume until `ts + 1`.

Getting these two cases right is the whole problem.

## Trace — Example 1 (`n=2`)

| log | action | stk | prev | ans |
|---|---|---|---|---|
| `0:start:0` | push 0 | `[0]` | 0 | `[0,0]` |
| `1:start:2` | `ans[0]+=2-0` ; push 1 | `[0,1]` | 2 | `[2,0]` |
| `1:end:5`   | `ans[1]+=5-2+1` ; pop | `[0]` | 6 | `[2,4]` |
| `0:end:6`   | `ans[0]+=6-6+1` ; pop | `[]` | 7 | `[3,4]` |

Result `[3, 4]`. ✅

## Complexity

- **Time** `O(L)` — one pass; each entry does O(1) work (parse + push/pop).
- **Space** `O(D + n)` — stack depth `D` plus the answer array.

## Follow-ups

- **Multi-threaded / concurrent functions** → the stack invariant breaks; track an interval
  per (function, thread) or use a sweep over events instead.
- **Total (inclusive) time instead of exclusive** → drop the "settle onto the top" step and
  just record each function's own `end - start + 1` intervals.
- **Malformed / unbalanced logs** → validate that each `end` matches the current top and that
  the stack is empty at the finish; reject otherwise.
