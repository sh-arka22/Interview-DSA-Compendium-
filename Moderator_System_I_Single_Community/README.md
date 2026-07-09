---
company: Reddit
difficulty: Medium
tags:
  - design
  - linked-list
  - hash-map
  - ranking
---

# Moderator System I — Single Community Ranking

**Company:** Reddit | **Difficulty:** Medium | **Role:** Software Engineer

## Problem

Drive a single-community moderator system from a chronological log of
`"<targetUser>,<action>,<actorUser>,<timestamp>"` records (`action` ∈ `{added, removed}`).
Moderators are ranked by when they most recently became a moderator — an earlier promotion
means a higher level.

`solution(logs, queries)`:

| Query | Encoding | Returns |
|---|---|---|
| Ranking | `["getModRanking"]` | list of moderators, highest → lowest |
| Permission check | `["canRemoveMod", targetUser, actorUser]` | `True` iff both are current mods, they differ, and `actorUser` outranks `targetUser` |

## Examples

```
logs = [alice added(system,1), bob added(alice,2), carol added(alice,3),
        bob removed(alice,4), bob added(carol,5)]

getModRanking()               -> [alice, carol, bob]
canRemoveMod(bob, alice)      -> True    // alice (ts1) outranks bob (ts5, re-added)
canRemoveMod(carol, bob)      -> False   // bob (ts5) ranks below carol (ts3)
canRemoveMod(alice, alice)    -> False   // same user
canRemoveMod(dave, alice)     -> False   // dave isn't a moderator

logs = []
getModRanking()                -> []
```

## Approach

A doubly linked list ordered highest→lowest rank, plus a hash map from user to list
iterator. See [ALGORITHM.md](ALGORITHM.md).

**Key insight:** logs are already given in strictly increasing timestamp order, so
processing them in order *is* chronological order — the timestamp field itself never needs
to be parsed or stored. A fresh `"added"` is always the newest promotion among currently
active moderators, so it always belongs at the list's tail: `push_back`, never
insert-and-sort.

## Edge Cases

- `canRemoveMod` with `targetUser == actorUser` → `False`, even for two top-ranked mods.
- Either user missing / not currently a moderator → `False`.
- Empty log → empty ranking, every `canRemoveMod` call is `False`.
- A user removed then re-added gets a **fresh, lowest** rank — their earlier promotion time
  is irrelevant once they've been removed.

## Complexity

- Construction: **O(N)** for N log lines (one hash-map + list op per line).
- `canRemoveMod`: **O(1)**.
- `getModRanking`: **O(k)**, k = current moderator count (unavoidable — must emit the whole list).
- Space: **O(k)**.
