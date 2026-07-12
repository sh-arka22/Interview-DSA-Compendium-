# Segment Tree — Iterative vs Recursive

Non-recursive (bottom-up) segment tree in ~15 lines, side by side with the
classic recursive version, plus a stress test proving both correct against
brute force and a micro-benchmark showing the constant-factor gap.

## Files

- `solution.cpp` — both implementations + demo + stress test + benchmark
- `ALGORITHM.md` — first-principles derivation, invariants, exact complexity, when to use which

## Build & run

```bash
g++ -O2 -std=c++17 solution.cpp -o solution && ./solution
```

## TL;DR

Both are O(n) build, O(log n) query/update. Iterative: 2n memory, ~2-4x
faster in practice, default interview choice for point-update problems.
Recursive: 4n memory, but required for lazy propagation, tree descent,
and persistent trees.
