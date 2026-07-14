---
tags:
  - algorithm
  - trie
  - hash-map
  - filesystem
related: "[[README]]"
---

# Algorithm — Trie of Hash Maps with a Tagged Node

## Why the obvious skeleton is slow

A very natural first draft (and the one most candidates sketch under time pressure)
gives each directory `Node` two parallel `vector`s — `vector<Node*> children_dir` and
`vector<File*> children_files` — and finds an entry by scanning:

```cpp
Node* findDir(Node* dir, const string& name) {
    for (auto* c : dir->children_dir) if (c->dirName == name) return c;
    return nullptr;
}
```

That's **O(fan-out)** per segment. Walking a path of depth `d` through directories
that each hold `f` entries costs O(d·f) — fine for the tiny inputs in the examples,
but it's doing linear work where a hash map does O(1). It also forces every function
to branch between "is this name a file or a directory?" using two separate lookups.

## The reframe: one Node type, one map, tagged by a boolean

Collapse "directory" and "file" into a single struct:

```cpp
struct Node {
    bool isFile = false;
    string content;                          // meaningful only when isFile
    unordered_map<string, Node*> children;    // name -> child, O(1) average
};
```

This is exactly a **trie**: the alphabet is "path segment names" instead of
characters, and each `Node` is a trie node whose child map is keyed by segment.
`isFile` is the tag that turns a plain trie into a filesystem — a leaf-capable node
can still (in principle) have children of its own if the problem allowed it, but here
files never do, so `isFile` is really "does this node carry content instead of
sub-entries."

## Two walk primitives, both O(depth)

```cpp
// creates missing directories as it goes — used by mkdir / addContentToFile
Node* walkOrCreate(parts, n) {
    Node* cur = root;
    for (i in [0, n)):
        find-or-emplace parts[i] in cur->children
        cur = that child
    return cur;
}

// assumes the path already exists — used by ls / readContentFromFile,
// which the problem guarantees are only ever called on valid paths
Node* walk(parts, n) {
    Node* cur = root;
    for (i in [0, n)): cur = cur->children[parts[i]];
    return cur;
}
```

Every one of the four public operations reduces to "walk to the right node, then do
one O(1)-ish thing":

| Op | Walk target | Then |
|---|---|---|
| `mkdir(path)` | `walkOrCreate(parts, parts.size())` | nothing — the walk itself created the chain |
| `addContentToFile(fp, content)` | `walkOrCreate(parts, parts.size()-1)` (parent dir) | find-or-create the file entry in `parent->children`, `+=` content |
| `readContentFromFile(fp)` | `walk(parts, parts.size()-1)` (parent dir) | return `parent->children[name]->content` |
| `ls(path)` | `walk(parts, parts.size())` (the target itself) | if `isFile`, return `{name}`; else collect + sort `children` keys |

## Why `ls` still needs a sort even though lookups are O(1)

`unordered_map` gives fast lookup but **no ordering guarantee** on iteration. The spec
requires lexicographic output, so `ls` always pays `O(k log k)` for the directory it
targets (`k` = that directory's own child count) — that cost is inherent to the
problem, not an artefact of the data structure, and it's strictly local to one
directory rather than the whole tree.

## Full trace — README example

```
mkdir("/a/b/c")
  walkOrCreate(["a","b","c"], 3):
    root.children: {}            -> create "a"  (Node, isFile=false)
    a.children:    {}            -> create "b"
    b.children:    {}            -> create "c"
  tree: root -> a -> b -> c

addContentToFile("/a/b/c/d", "hello")
  walkOrCreate(["a","b","c","d"], 3) -> lands on c (all three already exist, 3 hash hits)
  c.children: {} -> "d" missing -> create Node{isFile=true}
  d.content: "" + "hello" -> "hello"

ls("/")
  walk([], 0) -> root directly (empty parts, loop doesn't run)
  root.isFile == false -> collect root.children keys = ["a"] -> sort -> ["a"]

readContentFromFile("/a/b/c/d")
  walk(["a","b","c"], 3) -> c   (parent directory)
  c.children["d"]->content -> "hello"
```

## Complexity

- `mkdir`: O(d) average, d = path depth.
- `addContentToFile` / `readContentFromFile`: O(d + |content|) average.
- `ls`: O(d + k log k) average, k = child count of the target directory.
- Space: O(sum of all path-segment lengths + all stored file content).

## Follow-ups

- **Symlinks / hard links.** A child map entry could point to another `Node*` shared
  by two names; needs cycle detection on `ls`/walk if links can point at ancestors.
- **Permissions / ownership.** Add `uid`/`mode` fields to `Node`; check them in the
  walk before allowing a create/read.
- **Large files / streaming reads.** Swap `content` for a chunked buffer or an
  on-disk-backed blob reference so `readContentFromFile` doesn't force one giant
  contiguous string in memory.
- **Concurrent access.** Per-directory (per-`Node`) locks, or a copy-on-write trie for
  lock-free reads — the same pattern used in persistent-data-structure file systems.
- **Path caching.** If the same deep path is walked repeatedly, memoise the last
  resolved `Node*` per absolute path string (invalidate on any `mkdir`/delete under
  that prefix) to turn repeated O(d) walks into O(1).
