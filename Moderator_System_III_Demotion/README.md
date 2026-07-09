---
company: Reddit
difficulty: Hard
tags:
  - design
  - linked-list
  - hash-map
  - order-maintenance
  - ranking
---

# Moderator System III — Demotion

**Company:** Reddit | **Difficulty:** Hard | **Role:** Software Engineer

## Problem

Extends [Part II](../Moderator_System_II_Multiple_Communities/README.md) with manual
demotion. After a demotion, the ranking is driven by a **maintained order** (seeded from
promotion timestamps), not purely by timestamp.

`solution(logs, queries)` adds:

| Query | Encoding | Returns |
|---|---|---|
| Demote | `["demote", community, user]` | `None` — moves `user` down exactly one position; no effect if already lowest or not a current mod |

`getModRanking` / `canRemoveMod` are unchanged in signature but now read the **maintained**
order (positions), not raw promotion timestamps.

## Examples

```
order(c1) = [user1, user2, user3]
demote(c1, user1)   -> [user2, user1, user3]
demote(c1, user1)   -> [user2, user3, user1]

demote(c1, user1); demote(c1, user3)  // user3 already lowest -> no-op
getModRanking(c1)                      -> [user2, user1, user3]
demote(c1, ghost)                      // not a mod -> no-op
getModRanking(c1)                      -> [user2, user1, user3]
```

## Approach

Same list + hash map as Parts I–II. `demote` swaps two **adjacent** list nodes in O(1) via
`std::list::splice`, and swaps their rank keys at the same time so `canRemoveMod`'s O(1) key
comparison stays correct after the reorder. See [ALGORITHM.md](ALGORITHM.md).

## Edge Cases

- `demote` on a non-moderator, or on the community's current lowest-ranked moderator → no-op
  (still counts as one query with a `None` result).
- Demotions compound: two calls on the same user move them down two slots total, unless they
  hit the bottom first.
- A user removed and later re-added still gets a **fresh, lowest** rank — past demotions
  applied to *other* users don't resurrect anyone's old position.
- `canRemoveMod` and `getModRanking` must reflect demotions immediately and consistently with
  each other (both read the same maintained order).

## Complexity

- Construction: **O(N)**. `canRemoveMod`: **O(1)**. `demote`: **O(1)**.
- `getModRanking`: **O(k)**, k = moderators currently in that community.
- Space: **O(total active moderators across all communities)**.
