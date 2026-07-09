# Algorithm — Same-Domain Web Crawl (BFS)

A first-principles walkthrough, breadth-first: the broad shape first, then each detail.

## 1. What kind of problem is this?

Strip away the crawler framing and it is **reachability in a directed graph from a single source**,
reported in **level order**. That is exactly what BFS gives you. The graph is handed to you as an
adjacency list (`map<url, vector<url>>`), so there is no fetching — the entire problem is the
traversal plus two filters.

Why BFS and not DFS? The spec demands **BFS visitation order** ("when a page lists several new
links, enqueue them in the order they appear"). A FIFO queue processing links left-to-right
produces precisely that order. DFS would give a different, incorrect ordering.

## 2. The skeleton: queue-based BFS

```
seen = {seed}          # set of URLs already discovered
q    = [seed]          # FIFO queue
order = []             # output, in visitation order
while q not empty:
    u = q.pop_front()
    order.append(u)    # <-- we "visit" u here
    for v in neighbours(u):
        if v not yet accepted:
            seen.add(v)
            q.push_back(v)
```

Three invariants make this correct:

- **A node is appended to `order` when it is dequeued.** Dequeue order under a FIFO is level order,
  so `order` is the BFS visitation order.
- **`seen` is updated on enqueue.** This is the subtle bit — see §4.
- **Neighbours are scanned in listed order.** Preserves the "enqueue in the order they appear" rule.

## 3. The two filters

Onto that skeleton we bolt two conditions before a neighbour `v` is accepted:

**Same-domain.** Compute `target = root(host(seed))` **once**. Accept `v` only if
`root(host(v)) == target`. Cross-domain links are dropped *before* they enter the queue, so we never
traverse through them — a link on `other.com` cannot pull in `other.com`'s links.

**Dedupe.** `if v in seen: skip`. Duplicate links on a page, back-links to the seed, and diamonds in
the graph all collapse to a single enqueue.

Order of the two checks does not affect correctness (a URL must pass both). Checking `seen` first is
a hair cheaper — a hash lookup vs. two string parses — so we do that.

## 4. Why mark `seen` on enqueue, not on dequeue

This is the classic BFS bug. Consider the given example: both `example.com/` and `blog.example.com/x`
link to `example.com/b`.

- **Mark on enqueue (correct):** when the seed is processed, `/b` is not yet reachable from it; later
  `/a` enqueues `/b` and adds it to `seen`. When `blog…/x` is processed and also lists `/b`, the
  `seen` check skips it. `/b` is enqueued **once**.
- **Mark on dequeue (buggy):** `/a` enqueues `/b`; before `/b` is dequeued, `blog…/x` is processed
  and, seeing `/b` still unmarked, enqueues it **again**. `/b` appears twice in the output.

So the discipline is: *the moment you decide to enqueue a node, mark it seen.*

## 5. Parsing — host and root domain

Both parses avoid building intermediate containers; they are pure index arithmetic.

**host(url)** — text between `://` and the next `/`:

```
s = find("://");  s = (found ? s+3 : 0)     # tolerate a missing scheme
e = find('/', s)                            # next slash after the host
return url[s : e]                           # e == npos -> to end (no trailing slash)
```

Two edge cases folded in: no scheme (`s` starts at 0) and no trailing slash (`e == npos`, take the
rest of the string).

**root(host)** — last two dot-separated labels, via two right-anchored searches:

```
d1 = rfind('.')                 # last dot
if none: return host            # single label, defensive
d2 = rfind('.', d1 - 1)         # second-to-last dot
return none(d2) ? host : host[d2+1 :]
```

`blog.example.com`: `d1` before `com`, `d2` before `example` → returns `example.com`.
`example.com`: `d1` before `com`, no earlier dot → returns the whole thing.

This is O(host length) with zero allocations, versus splitting on `.` into a vector and re-joining
the tail. In a 30-minute interview either is acceptable; the two-`rfind` form is the tighter one.

## 6. Walking the given example

seed `https://example.com/`, `target = example.com`.

| step | dequeue | its links | accepted (root == example.com, unseen) | queue after |
|------|---------|-----------|----------------------------------------|-------------|
| 1 | `…/` | `/a`, `blog…/x`, `other.com/` | `/a`, `blog…/x` (drop `other.com`) | `/a`, `blog…/x` |
| 2 | `/a` | `…/` (seen), `/b` | `/b` | `blog…/x`, `/b` |
| 3 | `blog…/x` | `other.com/` (drop), `/b` (seen) | — | `/b` |
| 4 | `/b` | [] | — | — |

Output: `["https://example.com/", "https://example.com/a", "https://blog.example.com/x",
"https://example.com/b"]`. ✓

## 7. Complexity

- **Time:** `O(V + E)` for the traversal — each node dequeued once, each edge scanned once — times
  `O(L)` per URL for the host/root parse, i.e. `O((V + E)·L)`. Map lookups add an `O(log V)` factor;
  an `unordered_map<string, vector<string>>` would drop that to `O(1)` amortized, but the platform
  fixes the parameter type as `map`.
- **Space:** `O(V)` — the `seen` set, the queue, and the output vector each hold at most every
  same-domain URL once.

## 8. Interview talking points / follow-ups

- **Real crawler:** replace `graph[u]` with an async fetch + link extraction; parallelize with a
  worker pool over the frontier, guarding `seen` with a lock or using per-shard sets.
- **Multi-part suffixes (`co.uk`):** the two-label `root` rule breaks; use a Public Suffix List
  (e.g. Mozilla's) to find the registrable domain.
- **Normalization:** production code would canonicalize URLs (lowercase host, strip default ports,
  resolve `.`/`..`, unify trailing slashes) before dedupe — otherwise `a.com/` and `a.com` count as
  two pages. Here URLs are treated as opaque strings, matching the spec.
- **Politeness / scale:** rate limiting per host, robots.txt, and a bounded frontier for a web-scale
  crawl.
