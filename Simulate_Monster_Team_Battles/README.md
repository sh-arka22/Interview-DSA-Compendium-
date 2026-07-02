# Simulate Monster Team Battles

| Company | Difficulty | Tags |
|---------|-----------|------|
| Meta | Medium | Simulation · Object Modeling · Type Effectiveness · Turn Sequencing |

## Problem

Two monster teams battle. Simulate the fight turn-by-turn and return a log of all events.

### Models
- **Team**: `{name, monsters[]}`
- **Monster**: `{name, type, health, attacks[]}`
- **Attack**: `{name, type, damage}`

### Rules
1. Each team fields its **first alive** monster (in team order)
2. Turns alternate; **Team A always attacks first** in each new matchup
3. A monster picks the attack with highest `base_damage × type_multiplier`
4. Ties → earliest attack in the list wins
5. Missing type chart entries default to **1.0**
6. Health can't go below 0
7. Fainted monster (health ≤ 0) does **not** attack
8. When a monster faints → next alive enters → Team A attacks first
9. Battle ends when one/both teams have no alive monsters

### Output
```python
{'winner': 'team_a' | 'team_b' | 'draw',
 'battle_log': [attack_events..., faint_events...]}
```

## Example Walkthrough

**Team Alpha**: Blaze (fire, 30hp) with Flame(fire,10) and Bite(normal,8)  
**Team Beta**: Leafy (grass, 30hp) with Vine(grass,7)  
**Chart**: fire→grass = 2.0, grass→fire = 0.5

```
Turn 1: Blaze → Leafy
  Flame: 10 × 2.0 = 20.0  ← best
  Bite:   8 × 1.0 =  8.0
  Leafy: 30 - 20 = 10hp

Turn 2: Leafy → Blaze
  Vine: 7 × 0.5 = 3.5
  Blaze: 30 - 3.5 = 26.5hp

Turn 3: Blaze → Leafy
  Flame: 20.0 again
  Leafy: 10 - 20 = 0hp → FAINT
  Next monster (Rocko) enters → new matchup

Turn 4: Blaze → Rocko
  Flame: 10 × 0.5 = 5.0   (fire→rock penalty!)
  Bite:   8 × 1.0 = 8.0   ← best now
  ...battle continues...
```

## Constraints
- 0 ≤ monsters per team ≤ 100
- Each monster has ≥ 1 attack
- Monsters with health ≤ 0 at start are skipped silently

## Files
- [solution.cpp](./solution.cpp) — Full implementation with Python-style I/O parsing
- [ALGORITHM.md](./ALGORITHM.md) — Detailed algorithm breakdown
