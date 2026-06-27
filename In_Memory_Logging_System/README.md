---
company: Rippling
difficulty: Easy (Part 1) / Medium (Part 2)
round: Technical Screen
tags:
  - rippling
  - design
  - inverted-index
  - string-processing
algorithms:
  - Handler Pipeline (Chain of Responsibility)
  - Linear Scan Search
  - Inverted Index
---

# In-Memory Logging System

**Company:** Rippling | **Role:** Software Engineer | **Difficulty:** Easy → Medium | **Round:** Technical Screen

---

## Problem Statement

Implement an in-memory logger that supports three concepts:

### Handlers
Transform a log message before it is stored. Handlers are applied **in the order they were added**.

| Handler | Operation |
|---|---|
| `upper` | Convert entire message to uppercase |
| `prefix <text>` | Prepend `<text>` to the message |
| `suffix <text>` | Append `<text>` to the message |

### Logs
Each `add_log` message passes through all registered handlers, then the **transformed** message is stored with a unique, auto-incrementing id starting at **1**.

### Operations (input format)

| Operation | Arguments | Produces output? |
|---|---|---|
| `add_handler` | `type [text]` | No |
| `add_log` | `message` | No |
| `get_logs` | — | Yes — all stored logs in insertion order |
| `search` | `keyword` | Yes — all logs containing the keyword |

---

## Output Format

`get_logs` and `search` both return strings of the form `"id:transformed_message"`.

```
get_logs  →  ["1:HELLO WORLD DONE", "2:MIXED CASE DONE"]
search    →  ["1:HELLO WORLD DONE"]          ← only logs containing the exact word
```

---

## Part 1 (Easy) — Linear Scan Search

`search(keyword)` scans all stored logs; for each log, tokenize its message by whitespace and check whether `keyword` exactly matches any token. Matching is **case-sensitive**.

## Part 2 (Medium) — Inverted Index Search

Maintain an inverted index: `word → [log_id, ...]`. On `add_log`, tokenize the transformed message and record the log's id under each unique word. `search` becomes an O(K) index lookup instead of O(L·W) scan.

---

## Examples

### Example 1 (handlers + case-sensitivity)

```
Operations:
  add_handler  upper
  add_handler  suffix  " DONE"
  add_log      "hello world"           → pipeline: "hello world" → "HELLO WORLD" → "HELLO WORLD DONE"
  add_log      "mixed Case"            → pipeline: "mixed Case" → "MIXED CASE" → "MIXED CASE DONE"
  get_logs
  search       "HELLO"
  search       "hello"                 ← case-sensitive; no match
  search       "DONE"

Output:
  [ ["1:HELLO WORLD DONE", "2:MIXED CASE DONE"],
    ["1:HELLO WORLD DONE"],
    [],
    ["1:HELLO WORLD DONE", "2:MIXED CASE DONE"] ]
```

### Example 2 (prefix + suffix ordering)

```
Operations:
  add_handler  prefix  "pre "
  add_handler  suffix  " post"
  add_log      "core"                  → "pre core" → "pre core post"
  search       "pre"
  search       "post"
  get_logs

Output:
  [ ["1:pre core post"], ["1:pre core post"], ["1:pre core post"] ]
```

### Example 3 (Part 2 — duplicate word deduplication)

```
Operations:
  add_handler  upper
  add_log      "hello hello world"     → "HELLO HELLO WORLD"   (HELLO appears twice; indexed once)
  add_log      "world"                 → "WORLD"
  search       "HELLO"
  search       "WORLD"
  search       "hello"
  get_logs

Output:
  [ ["1:HELLO HELLO WORLD"],
    ["1:HELLO HELLO WORLD", "2:WORLD"],
    [],
    ["1:HELLO HELLO WORLD", "2:WORLD"] ]
```

### Example 4 (Part 2 — empty log and empty keyword)

```
Operations:
  add_log      ""                      → stored as empty string, id=1; no words indexed
  search       "anything"
  search       ""                      ← empty keyword always returns []
  get_logs

Output:
  [ [], [], ["1:"] ]
```

---

## Edge Cases

### Handlers

| # | Case | Why It Matters |
|---|---|---|
| 1 | **No handlers registered** | Message stored verbatim. `search` matches against the original message. |
| 2 | **Handler order is critical** | `[prefix("pre "), upper]` on "core" → "pre core" → "PRE CORE". `[upper, prefix("pre ")]` on "core" → "CORE" → "pre CORE". Results differ. |
| 3 | **Same handler added twice** | Both run: double-prefix gives `"pre pre core"`, double-upper is a no-op. |
| 4 | **Suffix/prefix WITH space** | `suffix(" END")` on "msg" → "msg END"; words are `{"msg", "END"}`. Searchable as "END". |
| 5 | **Suffix/prefix WITHOUT space** | `suffix("END")` on "msg" → "msgEND"; only word is "msgEND". `search("END")` returns nothing — not an exact word. |
| 6 | **Empty prefix/suffix text** | Concatenating "" is a no-op; message unchanged. |
| 7 | **Upper after prefix** | Uppercases the ENTIRE string including the prefix text added in the previous step. |

### Logs and Search

| # | Case | Why It Matters |
|---|---|---|
| 8 | **Empty message** | Stored as empty string; `get_logs` returns `"id:"`. No words → never returned by `search`. |
| 9 | **Empty keyword** | `search("")` always returns `[]`. Guard this before any index lookup. |
| 10 | **Case sensitivity** | `search("hello")` ≠ `search("HELLO")`. Upper handler produces uppercase; search must match exactly. |
| 11 | **Substring matching is wrong** | "HELL" must NOT match a log containing "HELLO". Search compares whole whitespace-separated tokens only. |
| 12 | **Repeated word in one log** | "foo foo foo" — `search("foo")` returns that log ONCE, not three times. Both linear scan (break after first match) and index (deduplicate on insert) must handle this. |
| 13 | **Keyword in no log** | Returns `[]`. Index has no entry (inverted) or scan finds nothing (linear). |
| 14 | **get_logs when empty** | Returns `[]`. |
| 15 | **Multiple spaces between words** | `"hello   world"` → tokens "hello", "world" (stream extraction skips whitespace). The stored message still has triple-space but tokenization is correct. |
| 16 | **Search results in insertion order** | Both methods return logs in ascending id order. Index stores ids in insertion order; linear scan iterates in order. |
| 17 | **All logs match keyword** | All are returned, all in insertion order. |
| 18 | **k changes dynamically (follow-up)** | N/A here — k is fixed per session. |

---

## Constraints

- `0 ≤ len(operations) ≤ 10,000`
- `0 ≤ len(message), len(text), len(keyword) ≤ 1,000`
- Total length of all raw log messages ≤ 1,000,000
- Log ids start at 1 and increase by 1 per `add_log`.
- Search is case-sensitive and matches exact whitespace-separated words on the **transformed** message.
- Repeated words in one log must not cause duplicate results.

---

## Algorithm

See [ALGORITHM.md](./ALGORITHM.md) for full details.

**Algorithms Used:**
- **Handler Pipeline** (Chain of Responsibility) · **Linear Scan Search** (Part 1) · **Inverted Index** (Part 2)
