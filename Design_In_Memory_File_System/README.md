---
tags:
  - interview
  - design
  - trie
  - hash-map
  - filesystem
  - simulation
---

# Design In-Memory File System

**Difficulty:** Hard · **Time:** 40 min · **Pattern:** LeetCode 588 (Design In-Memory File System)

## Problem

Design an in-memory file system supporting:

| Operation | Behaviour |
|---|---|
| `ls(path)` | File path → list containing just that file's name. Directory path → names of its immediate children (files + directories together), **lexicographically sorted**. |
| `mkdir(path)` | Create the directory, creating every missing middle directory along the way. |
| `addContentToFile(filePath, content)` | Create the file with `content` if it doesn't exist; otherwise **append** `content` to what's already there. |
| `readContentFromFile(filePath)` | Return the file's full content. |

Constraints: lowercase names only, no name collisions within a directory, all calls use valid/existing paths (no need to guard missing paths on read), ≤300 total calls.

### Example

```
fs.ls("/")                              -> []
fs.mkdir("/a/b/c")
fs.addContentToFile("/a/b/c/d", "hello")
fs.ls("/")                              -> ["a"]
fs.readContentFromFile("/a/b/c/d")      -> "hello"
```

## Approach — trie of hash maps, one Node type for both file and directory

See [ALGORITHM.md](ALGORITHM.md) for the full derivation. In short:

- The file system **is** a trie: each path segment is one hop to a child `Node`, keyed
  by name in an `unordered_map<string, Node*>` — **O(1) average** per segment instead
  of the linear vector scan a naive `vector<Node*>` design forces.
- A single `Node` struct doubles as directory **and** file via an `isFile` flag, so
  `mkdir`/`addContentToFile`/`ls` share one child map and one walk routine — no
  parallel `vector<File*>` / `vector<Node*>` bookkeeping, no duplicated lookup code.
- `addContentToFile` is create-or-append in one code path: look the name up, create a
  file `Node` on miss, then always `+=` the content.

## Edge Cases

- **`ls` on a file path** — must return `{filename}`, not the parent directory's
  listing. Checked via the `isFile` flag before touching `children`.
- **Repeated `addContentToFile` on the same file** — must *append*, never overwrite.
- **`mkdir`/`addContentToFile` on paths sharing a prefix** — middle directories are
  created once (`emplace` on first miss) and reused by every subsequent path through
  them, never duplicated.
- **`addContentToFile` on a path with no prior `mkdir`** — every missing middle
  directory must be created implicitly, exactly like `mkdir` does.
- **`ls` output ordering** — files and directories are sorted together in one
  lexicographic pass, not grouped "dirs first" or "files first".
- **Root `ls("/")`** — `parse` returns an empty segment list; the walk loop simply
  never executes and returns `root` directly, so root needs no special-casing.

## Complexity

Let `d` = number of path segments (depth) and `k` = number of children in the
directory an `ls` targets.

- `mkdir`: **O(d)** average (one hash find/emplace per segment).
- `addContentToFile`: **O(d + |content|)** average.
- `readContentFromFile`: **O(d + |content|)** average.
- `ls`: **O(d + k log k)** — the walk is O(d); the sort is unavoidable since the
  output must be lexicographic.
- Space: **O(total characters across all created paths and file contents)**.
