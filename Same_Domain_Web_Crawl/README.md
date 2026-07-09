---
tags:
  - interview
  - graph
  - bfs
  - strings
---

# Same-Domain Web Crawl (BFS)

**Company:** Anthropic · **Difficulty:** Medium

## Problem

You are given a snapshot of a small portion of the web as a **directed graph**: a map from each
page URL to the list of URLs it links to. Starting from a `seed` URL, return **all pages reachable
from the seed that share the seed's root domain**, discovered via **breadth-first search**, with
duplicates removed.

The real crawler would fetch each page over the network (and parallelize with asyncio/aiohttp);
here the link graph is given directly, so the focus is the traversal, the de-duplication, and the
same-domain filter.

### Definitions

- **host** of a URL = the substring between `://` and the next `/` (or end of string).
  `https://blog.example.com/posts/1` → host `blog.example.com`.
- **root domain** of a host = its **last two** dot-separated labels.
  `blog.example.com` → `example.com`; `example.com` → `example.com`.
  (Simple two-label roots only — no multi-part public suffixes like `co.uk`.)
- Two URLs are **same-domain** if their hosts have the same root domain.

### Input / Output

- `seed`: a URL string.
- `graph`: `map<string, vector<string>>` from a page URL to the URLs on it. A URL that appears as a
  link but not as a key has **no outgoing links**. Links may point to other domains (filtered out,
  not traversed) and may contain duplicates.
- **Return** `vector<string>`: every distinct same-domain URL reachable from `seed` (including the
  seed), in **BFS visitation order** starting from `seed`. When a page lists several new links,
  enqueue them in the order they appear.

### Examples

| `seed` | graph (abridged) | Output | Why |
|---|---|---|---|
| `https://example.com/` | seed → `/a`, `blog.example.com/x`, `other.com/`; `/a` → seed, `/b`; `blog…/x` → `other.com/`, `/b`; `/b` → [] | `["…/", "…/a", "blog…/x", "…/b"]` | `other.com` dropped (diff root); `blog.example.com` shares root `example.com` so kept; second `/b` and back-link to seed not revisited. |
| `https://foo.com/` | `{}` | `["https://foo.com/"]` | Seed is not a key → no outgoing links → only the seed is reachable. |

## Core idea — textbook BFS + two filters

This is a standard queue-based BFS over a graph. Two things are layered on top:

1. **Same-domain filter** — compute the seed's root domain once; keep a link only if its own root
   domain matches.
2. **De-duplication** — a `visited` set so each URL enters the queue at most once.

The one detail that must be exact: **mark a node visited on ENQUEUE, not on dequeue.** A URL linked
from two pages (e.g. `/b`) would otherwise be pushed twice. Recording the output on *dequeue* gives
the correct BFS visitation order.

```
target = root(host(seed))
seen   = {seed};  q = [seed]
while q:
    u = q.pop_front();  order.push_back(u)      # visitation order
    for v in graph[u]:                          # links in listed order
        if v in seen: continue                  # dedupe
        if root(host(v)) != target: continue    # same-domain filter
        seen.add(v);  q.push_back(v)            # mark on enqueue
```

### Parsing the host and root

- `host`: find `://`, take from there to the next `/`. Handle *no scheme* (start at 0) and *no
  trailing slash* (take to end).
- `root`: two `rfind('.')` calls from the right grab the last two labels in O(length) with no
  intermediate vector — cheaper than split-and-join.

## Edge Cases

- **Seed absent as a key** — `graph.find` misses → the loop over links never runs → output is just
  the seed (Example 2).
- **Duplicate links** — the same URL listed twice on a page is enqueued once (`seen` guards it).
- **Cross-domain links** — dropped *before* enqueue, so they are never traversed even transitively.
- **Back-link to the seed / already-seen nodes** — skipped by `seen`.
- **Subdomains** — `shop.a.com` and `a.com` share root `a.com`; a subdomain seed still matches the
  bare domain and vice-versa.
- **No trailing slash** (`https://a.com`) — host parsing runs to end of string, still yields
  `a.com`.
- **Single-label host** (no dot) — `root` returns the host unchanged (defensive; not in the given
  constraints).

## Complexity

- **Time:** `O(V + E)` traversal; each URL's `host`/`root` parse is `O(L)` in URL length. Map
  lookups are `O(log V)` (the signature fixes the container as `map`; an `unordered_map` would make
  them `O(1)`).
- **Space:** `O(V)` for the `visited` set, the queue, and the output.

## Files

- `solution.cpp` — the `Solution` class (matches the platform signature).
- `test.cpp` — self-checking tests; `#include`s `solution.cpp`.
- `ALGORITHM.md` — deeper first-principles walkthrough.

Run the tests:

```
g++ -std=c++17 -O2 -o test test.cpp && ./test
```
