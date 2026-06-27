---
company: Rippling
tags:
  - algorithm
  - inverted-index
  - pipeline
  - design
related: "[[README]]"
---

# Algorithm: In-Memory Logging System

---

## Part 1 — Handler Pipeline + Linear Scan

### Handler Pipeline (Chain of Responsibility)

Store handlers as a polymorphic list. Each `add_log` call passes the message through every handler in order:

```
transformed = rawMessage
for each handler in pipeline:
    transformed = handler.apply(transformed)
store(transformed)
```

**Why polymorphism over if-chains?**  
A new handler type (e.g., `LowerHandler`, `TrimHandler`) is added by subclassing — no change to the dispatch loop. In an interview, mentioning this trade-off (code cleanliness vs. slightly higher overhead) shows design awareness.

**Handler implementations:**

| Handler | apply(msg) |
|---|---|
| `UpperHandler` | `std::transform` with `::toupper` on each character |
| `PrefixHandler(text)` | `return text + msg` |
| `SuffixHandler(text)` | `return msg + text` |

### Log Storage

`std::vector<std::string> logs_` stores transformed messages in insertion order. The log id is `index + 1` (zero-based vector, one-based ids). This gives:
- `add_log`: O(M·H) — M characters, H handlers (each O(M) for string concat/transform)
- `get_logs`: O(L) to build "id:msg" strings for all L logs
- `logs_[id-1]`: O(1) lookup by id

### Linear Scan Search (Part 1)

```
search(keyword):
    if keyword is empty: return []
    result = []
    for i in 0..L-1:
        tokenize logs_[i] by whitespace
        if keyword in tokens:
            result.append(str(i+1) + ":" + logs_[i])
            break  ← stop tokenizing this log; it's already in results
    return result
```

**Deduplication in linear scan:** The `break` after the first match guarantees each log appears at most once, even if the keyword appears multiple times within it.

**Complexity:** O(L·W) per search — L logs, W average words per log.

---

## Part 2 — Inverted Index Search

### Index Structure

```
std::unordered_map<std::string, std::vector<int>> index_
// word → list of log ids (ascending, no duplicates)
```

Because log ids are assigned in increasing order and we only push to a word's list during `add_log`, each word's id list is already sorted in insertion order. No explicit sort needed.

### Building the Index on add_log

```
add_log(rawMsg):
    transformed = applyHandlers(rawMsg)
    id = logs_.size() + 1
    logs_.push_back(transformed)
    
    seen = {}                     ← per-log deduplication set
    for word in tokenize(transformed):
        if word not in seen:
            seen.add(word)
            index_[word].append(id)  ← each id appears at most once per word
```

**Why deduplicate with a local `seen` set?**  
Without it, a log like "HELLO HELLO WORLD" would add id 1 twice to `index_["HELLO"]`. The search result would then list log 1 twice for "HELLO" — which violates the requirement.

The `seen` set is local to each `indexLog` call: it tracks uniqueness within a single log, not across logs.

### Inverted Index Search

```
search(keyword):
    if keyword is empty: return []
    if keyword not in index_: return []
    ids = index_[keyword]                     ← already in insertion order
    return ["id:logs_[id-1]" for id in ids]
```

**Complexity:** O(1) hash lookup + O(K) result construction — K = number of matching logs. For L total logs and K matches, this is O(K) vs O(L·W) for linear scan.

---

## Tokenization

Both search methods use the same tokenization rule: split by whitespace using `std::istringstream` with `>>`.

```cpp
std::istringstream iss(msg);
std::string word;
while (iss >> word) {
    // process word
}
```

**Behavior:**
- `"hello world"` → `["hello", "world"]`
- `"hello   world"` → `["hello", "word"]` (multiple spaces collapsed)
- `""` → `[]` (no tokens; the log is stored but never returned by search)
- `"  "` → `[]` (whitespace only; no tokens)

The stored message retains its original whitespace. Only the tokenization for indexing/matching collapses it.

---

## Complexity Summary

| Operation | Part 1 (linear) | Part 2 (inverted index) |
|---|---|---|
| `add_handler` | O(1) | O(1) |
| `add_log` (M chars, W words, H handlers) | O(M·H) | O(M·H + W) |
| `get_logs` | O(L) | O(L) |
| `search` | O(L·W) | O(1 + K) |
| Space | O(L·M) | O(L·M + V) where V = vocabulary size |

**When does Part 2 matter?**  
When L (logs) is large but K (matching logs) is small. A keyword appearing in only 3 of 10,000 logs costs O(3) with the index vs O(10,000 × avg_words) with scan.

**Trade-off:** Part 2 spends extra time and memory on index construction (`add_log` is slightly heavier). If searches are rare and L is small, Part 1's simplicity wins.

---

## Data Structure Choices

| Structure | Used for | Why |
|---|---|---|
| `vector<unique_ptr<Handler>>` | Handler pipeline | Ordered, polymorphic; O(1) append; O(H) traversal |
| `vector<string>` | Log store | Insertion-ordered; O(1) append; O(1) id→message lookup |
| `unordered_map<string, vector<int>>` | Inverted index | O(1) average lookup by word; word lists stay in insertion order |
| `unordered_set<string>` | Per-log deduplication | O(1) average insert/lookup; local to each add_log call |

---

## Design Extensions (Follow-up Discussion)

### Delete a log
- Not directly supported by this design.
- **Option A:** Mark log as deleted (tombstone flag). `get_logs` and `search` skip tombstoned entries. O(1) delete, O(L) extra scan work.
- **Option B:** Remove from index (must iterate all words in the log → O(W) per delete). Gaps in id sequence; rebuild `get_logs` to skip gaps.

### Case-insensitive search
- At index time, normalize each word to lowercase before inserting into the index.
- At query time, lowercase the keyword before lookup.
- The stored message stays as-is (case preserved for display); only the index uses normalized keys.

### Phrase search ("hello world" as a two-word phrase)
- Single-word inverted index is insufficient.
- Need positional index: `word → [(log_id, position), ...]` or an n-gram index.
- Merge candidate ids from each term, then verify positional adjacency.

### Multi-word AND/OR queries
- For AND: intersect the id lists of each keyword.
- For OR: union the id lists.
- Both O(K₁ + K₂) with sorted id lists and a merge step.
