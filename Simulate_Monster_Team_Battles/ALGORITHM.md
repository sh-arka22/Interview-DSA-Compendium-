# Algorithm: Simulate Monster Team Battles

---

## Core Idea

This is a **stateful simulation** — no clever algorithmic trick, just model the battle
rules precisely. The "optimization" is in clean data structures that make each step O(1)
or O(A) where A is the number of attacks a monster has.

---

## Data Structures

| Structure | Purpose |
|---|---|
| `Monster { name, type, health, attacks[] }` | Mutable — health decreases as battle proceeds |
| `Attack { name, type, damage }` | Immutable — base damage before type multiplier |
| `chart: HashMap<"atkType\|defType", double>` | O(1) type multiplier lookup, default 1.0 |
| `idxA, idxB` | Index of each team's current alive monster |
| `turnA: bool` | Whose turn it is (resets to `true` on new matchup) |

---

## Simulation Loop

```
while both teams have alive monsters:
    attacker = current team's monster (based on turnA)
    defender = opponent's current monster

    ── ATTACK SELECTION ──────────────────────────────
    for each attack in attacker.attacks:
        effective = attack.damage × mult(attack.type, defender.type)
    pick max effective; earliest on tie

    ── APPLY DAMAGE ──────────────────────────────────
    defender.health -= effective_damage
    clamp to 0

    ── LOG ATTACK EVENT ──────────────────────────────

    ── CHECK FAINT ───────────────────────────────────
    if defender.health == 0:
        log faint event
        advance defender's team to next alive monster
        turnA = true          ← new matchup, Team A goes first
    else:
        turnA = !turnA        ← alternate turns
```

---

## Key Rules to Get Right

| Rule | How it's handled |
|---|---|
| Team A attacks first in every **new matchup** | `turnA = true` after any faint |
| Highest effective damage wins | `base × multiplier`, scan all attacks |
| Tie-break: earliest attack | Linear scan, only update on **strictly greater** |
| Health ≤ 0 at start → skip silently | `nextAlive()` skips them; no faint event logged |
| Missing type chart entry → 1.0 | `chart.find()` miss returns 1.0 |
| Health can't go below 0 | `if (health < 0) health = 0` |

---

## Complexity

| Aspect | Cost |
|---|---|
| **Per turn** | O(A) to pick the best attack |
| **Total turns** | O(Σ health / min_damage) — bounded by total HP pool |
| **Type lookup** | O(1) hash map |
| **Overall** | O(T × A) where T = total turns, A = max attacks per monster |

With ≤ 100 monsters per team and reasonable health/damage values, this runs instantly.

---

## Parsing

The inputs are **Python dict strings** (single quotes, tuple keys for the type chart).
A simple recursive descent parser handles:
- `'string'` → extract content between quotes
- `30` / `2.0` → parse as double
- `('fire', 'grass')` → parse tuple as a pair of strings for chart key
- `{key: value, ...}` → dict
- `[item, ...]` → array

---

## Output Format

Returns a Python-style dict string:
```python
{'winner': 'team_a', 'battle_log': [
    {'event': 'attack', 'attacker_team': '...', 'attacker': '...', 
     'defender_team': '...', 'defender': '...', 'attack': '...', 
     'damage': 20.0, 'defender_health': 10.0},
    {'event': 'faint', 'team': '...', 'monster': '...'},
    ...
]}
```
