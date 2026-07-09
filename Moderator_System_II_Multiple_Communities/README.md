---
company: Reddit
difficulty: Medium
tags:
  - design
  - linked-list
  - hash-map
  - ranking
  - multi-tenant
---

# Moderator System II — Multiple Communities

**Company:** Reddit | **Difficulty:** Medium | **Role:** Software Engineer

## Problem

Extends [Part I](../Moderator_System_I_Single_Community/README.md): moderator status is now
scoped **per community**. Each log record gains a leading community field —
`"<community>,<targetUser>,<action>,<actorUser>,<timestamp>"` — and every query names a
community. Rankings are computed within a single community only; a user may moderate several
communities independently, with independent ranks in each.

`solution(logs, queries)`:

| Query | Encoding | Returns |
|---|---|---|
| Ranking | `["getModRanking", community]` | ranking for that community (`[]` if none / unseen) |
| Permission check | `["canRemoveMod", community, targetUser, actorUser]` | same rule as Part I, restricted to that community |

## Examples

```
logs = [c1:alice added(system,1), c1:bob added(alice,2), c2:bob added(system,3),
        c1:carol added(alice,4), c1:bob removed(alice,5), c1:bob added(carol,6)]

getModRanking(c1)                  -> [alice, carol, bob]
getModRanking(c2)                  -> [bob]
canRemoveMod(c1, bob, alice)       -> True
canRemoveMod(c1, carol, bob)       -> False
canRemoveMod(c2, bob, alice)       -> False   // alice isn't a mod of c2

logs = []
getModRanking(nope)                 -> []      // unknown community
```

## Approach

Same doubly linked list + hash map from Part I, just namespaced by community. See
[ALGORITHM.md](ALGORITHM.md).

## Edge Cases

- Unknown community → empty ranking / `False`, never throws.
- Same user, different communities → fully independent ranks (a user promoted first in `c1`
  can be promoted last in `c2`).
- Timestamps are globally increasing across communities, but each community keeps its own
  append counter, so cross-community timestamps never need to be compared.

## Complexity

- Construction: **O(N)** for N log lines (one extra hash lookup per line vs. Part I, to find
  the community).
- `canRemoveMod`: **O(1)**.
- `getModRanking`: **O(k)**, k = moderators currently in that community.
- Space: **O(total active moderators across all communities)**.
