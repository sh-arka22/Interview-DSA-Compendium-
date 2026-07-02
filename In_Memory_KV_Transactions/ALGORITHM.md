# Algorithm — Single Live State + Per-Transaction Undo Logs

## Why not a stack of maps?

The instinctive design is one hash map per open transaction, stacked. A `GET` then searches
top-down through the stack (O(depth)), and `COUNT` must scan every key of every layer (O(N)).
Both violate the "reads should be cheap / `COUNT` efficient" requirement.

The reframe: **apply every write immediately to a single `live` map**, and separately remember
*how to undo* each write. Reads become a plain O(1) lookup; rollback replays an undo log.

## The three structures

```
live       : key   -> value        the ONE visible state (base + all open txn writes folded in)
valueCount : value -> #keys         maintained incrementally  => COUNT is O(1)
frames     : stack of undo logs     frames.size() == current nesting depth
```

An undo record captures a key's state *before* a write:

```cpp
struct UndoRec { string key; bool hadValue; string prevValue; };
// hadValue=false  => "key did not exist before"
// hadValue=true   => "key existed and held prevValue"
```

## The single mutator: `applyChange(key, newHasValue, newValue)`

Every state change — a real `SET`, a real `DELETE`, and every rollback restore — flows through
here, which is what keeps `live` and `valueCount` in lockstep:

```
if key currently in live:
    decrement valueCount[old value]; erase that entry if it hits 0
if newHasValue:  live[key] = newValue;  valueCount[newValue]++     // a SET
elif key existed: live.erase(key)                                   // a DELETE
```

Invariant: `valueCount[v]` always equals the exact number of keys in `live` mapping to `v`, and
values with zero holders are absent from the map (so `COUNT` of a dead value returns 0).

## The journaler: `recordUndo(key)`

Called **before** `applyChange`, so it snapshots the pre-write state:

```
if frames empty: return                      // no open txn => write is permanent, don't journal
push {key, key∈live, live[key]} onto frames.back()   // record into the INNERMOST txn only
```

Two design points fall out of this:
1. Writes **outside** any transaction leave no journal (correct — they can't be rolled back).
2. Journaling always targets `frames.back()`, the transaction a `ROLLBACK` would undo.

## Command dispatch

| Command | Action |
|---|---|
| `SET k v` | `recordUndo(k)` → `applyChange(k, true, v)` |
| `DELETE k` | if `k ∈ live`: `recordUndo(k)` → `applyChange(k, false, "")` (else nothing) |
| `GET k` | output `live[k]` or `"NULL"` |
| `COUNT v` | output `valueCount[v]` or `0` |
| `BEGIN` | `frames.push_back({})` |
| `COMMIT` | empty → `"NO TRANSACTION"`; else `frames.clear()` |
| `ROLLBACK` | empty → `"NO TRANSACTION"`; else replay `frames.back()` reversed, then `pop_back()` |

**Why COMMIT is just `frames.clear()`:** the writes are *already* in `live`. Committing only
means "I no longer need to undo them," so dropping every undo log makes them permanent — and it
flattens *all* nesting levels at once, matching "apply everything since the outermost BEGIN."

## Why ROLLBACK replays in reverse

If a key is written twice in one transaction, the frame holds two records for it. Reverse replay
applies the *later* record first and the *earliest* (true original) last, so it wins.

```
BEGIN;  SET a 1;  SET a 2;         frame = [ {a, absent}, {a, was 1} ]
ROLLBACK replays reverse:
    apply {a, was 1}  -> a = 1
    apply {a, absent} -> a erased      final: a absent  ✓  (forward order would leave a=1, wrong)
```

## Full trace — Example 1

`live`=L, `valueCount`=VC, `frames`=F.

```
SET a 10   (F empty, no journal)      L{a:10}          VC{10:1}   F[]
GET a  -> "10"
BEGIN                                 L{a:10}          VC{10:1}   F[ [] ]
SET a 20  journal {a,was 10}          L{a:20}          VC{20:1}   F[ [{a:10}] ]
BEGIN                                 L{a:20}          VC{20:1}   F[ [{a:10}], [] ]
DELETE a  journal {a,was 20}          L{}              VC{}       F[ [{a:10}], [{a:20}] ]
GET a  -> "NULL"
ROLLBACK  restore a=20; pop           L{a:20}          VC{20:1}   F[ [{a:10}] ]
GET a  -> "20"
COMMIT    frames.clear()              L{a:20}          VC{20:1}   F[]
GET a  -> "20"
out = ["10","NULL","20","20"]  ✓
```

## Complexity

- `SET`/`DELETE`: O(1) amortized · `GET`/`COUNT`: O(1) · `BEGIN`: O(1).
- `ROLLBACK`: O(records in innermost frame) · `COMMIT`: O(total journaled records).
- Total: O(number of commands). Space: O(distinct keys + outstanding journaled writes).

## Follow-ups

- **`SAVEPOINT name` / `ROLLBACK TO name`.** Tag frames with names; roll back and pop frames
  until the named one. The undo-log model extends directly.
- **Durability / crash recovery.** Append each committed write to an on-disk log and replay on
  startup — see the sibling `Persistent_LRU_Cache` problem for the log-recovery pattern.
- **Concurrent transactions (not just nested).** Needs per-transaction *private* overlays plus a
  conflict/isolation policy (MVCC or locking) — a bigger design than a single-writer stack.
- **Range/prefix queries.** Swap the hash maps for an ordered map (`std::map`) to support
  ordered scans, trading O(1) reads for O(log n).
