---
company: DoorDash
difficulty: Medium
round: Technical Screen
tags:
  - doordash
  - medium
  - validation
  - hash-map
algorithms:
  - Two-Pass Validation
  - Aggregation with Hash Map
---

# Validate a Shopping Cart

**Company:** DoorDash | **Role:** Software Engineer | **Difficulty:** Medium

---

## Problem

Validate a customer's cart before checkout. Collect **all** errors — do not stop at the first.

**Cart item fields:** `itemId`, `quantity`
**Catalog item fields:** `itemId`, `availableQuantity`, `minQuantity`, `maxQuantity`

**Validation rules (in check order):**
1. Cart must not be empty.
2. Every item must have a non-empty `itemId`.
3. Every item must have a `quantity`.
4. `quantity` must be a positive integer (`> 0`; booleans are **not** valid integers).
5. `itemId` must exist in the catalog.
6. Referenced catalog items must have all three numeric fields as integers.
7. Duplicate `itemId`s in the cart have their quantities **aggregated** before rules 8–10.
8. Aggregated total must not exceed `availableQuantity`.
9. Aggregated total must be ≥ `minQuantity`.
10. Aggregated total must be ≤ `maxQuantity`.

**Return:** `{isValid, errors}` — errors in order: per-line errors by cart index first, then aggregate errors by first appearance of each item.

---

## Error Codes

| Code | Level | Fields |
|---|---|---|
| `EMPTY_CART` | global | `{code}` |
| `MISSING_ITEM_ID` | per-line | `{index, code}` |
| `MISSING_QUANTITY` | per-line | `{index, itemId, code}` |
| `INVALID_QUANTITY` | per-line | `{index, code}` |
| `ITEM_NOT_FOUND` | per-line | `{index, itemId, code}` |
| `INVALID_CATALOG_ITEM` | per-line | `{index, itemId, code}` |
| `EXCEEDS_AVAILABLE` | aggregate | `{itemId, code}` |
| `BELOW_MIN_QUANTITY` | aggregate | `{itemId, code}` |
| `EXCEEDS_MAX_QUANTITY` | aggregate | `{itemId, code}` |

---

## Examples

**Example 1 — Valid:**
```
cart:    [{burger, qty:2}, {fries, qty:1}]
catalog: [{burger, avail:5, min:1, max:3}, {fries, avail:10, min:1, max:5}]
→ {isValid: true, errors: []}
```

**Example 2 — Multiple errors:**
```
cart:    [{burger, qty:4}, {soda, qty:0}, {pizza, qty:1}]
catalog: [{burger, avail:3, min:1, max:5}, {soda, avail:10, min:1, max:6}]
→ errors:
    {index:1, code:"INVALID_QUANTITY"}          ← soda qty=0 not positive
    {index:2, itemId:"pizza", code:"ITEM_NOT_FOUND"}
    {itemId:"burger", code:"EXCEEDS_AVAILABLE"} ← total 4 > avail 3
```

---

## Key Edge Cases

| # | Case | Why It Matters |
|---|---|---|
| 1 | **Empty cart** | Rule 1; return immediately with `EMPTY_CART`. |
| 2 | **`quantity: 0` or boolean `"True"`** | `INVALID_QUANTITY`; not a positive integer. |
| 3 | **Duplicate `itemId`** | Quantities are summed first, then checked against catalog limits. One line being invalid doesn't prevent the other from contributing. |
| 4 | **Item exists in catalog but catalog fields are missing/non-integer** | `INVALID_CATALOG_ITEM` per-line; item doesn't contribute to aggregate total. |
| 5 | **Same item with one valid + one invalid quantity line** | Invalid line generates `INVALID_QUANTITY`; valid line still aggregates and may trigger `EXCEEDS_AVAILABLE`. |
| 6 | **Error ordering** | Per-line errors (in cart order) must come before aggregate errors (in first-seen item order). |

---

## Complexity

- **Time:** O(C + N) — C = cart size, N = catalog size
- **Space:** O(N) for catalog map + O(C) for aggregation

See [ALGORITHM.md](./ALGORITHM.md)
