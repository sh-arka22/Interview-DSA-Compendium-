---
company: DoorDash
tags:
  - algorithm
  - two-pass
  - validation
related: "[[README]]"
---

# Algorithm: Validate a Shopping Cart

## Approach: Two-Pass Validation

**Why two passes?** Rules 1–7 are per-line checks (can run as we scan the cart). Rules 8–10 require the aggregated total across all lines for a given `itemId`, so they must run after we've seen every cart line.

---

### Pass 1 — Per-line checks (in cart order)

For each cart line at index `i`:
1. No `itemId` → `MISSING_ITEM_ID`, skip line.
2. No `quantity` → `MISSING_QUANTITY`, skip line.
3. `quantity` not a positive integer → `INVALID_QUANTITY`, skip line.
4. `itemId` not in catalog → `ITEM_NOT_FOUND`, skip line.
5. Catalog entry missing required numeric fields → `INVALID_CATALOG_ITEM`, skip line.
6. Otherwise: `totals[itemId] += qty`; record first-seen order if new.

Skipping on error means invalid lines don't pollute the aggregate totals.

### Pass 2 — Aggregate checks (in first-seen order)

For each `itemId` that had at least one valid line:
```
if total > availableQuantity → EXCEEDS_AVAILABLE
else if total < minQuantity  → BELOW_MIN_QUANTITY
else if total > maxQuantity  → EXCEEDS_MAX_QUANTITY
```

Checked with `else-if` because only one aggregate condition can be the reported violation (over-available implies over-max is redundant; under-min and over-available can't coexist).

---

## Data Structures

| Structure | Purpose |
|---|---|
| `unordered_map<string, CatalogItem>` | O(1) lookup by itemId |
| `unordered_map<string, int>` totals | Accumulated quantity per itemId |
| `vector<string>` order | First-seen order of valid items for deterministic aggregate error output |
| `unordered_set<string>` seen | O(1) check for first occurrence |

---

## Error Ordering

The problem requires **deterministic** output:
1. All per-line errors in ascending `index` order (naturally produced by scanning cart[0..N-1]).
2. All aggregate errors in the order the `itemId` first appeared in the cart.

The `order` vector (populated in pass 1) guarantees rule 2 without sorting.

---

## Complexity

- O(N) to build catalog map.
- O(C) for pass 1 (C = cart items).
- O(K) for pass 2 (K = unique valid itemIds ≤ C).
- **Total: O(C + N) time, O(C + N) space.**
