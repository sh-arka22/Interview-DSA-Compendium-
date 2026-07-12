// ============================================================================
// Segment Tree: Iterative (bottom-up) vs Recursive (top-down)
// The iterative version is THE one to write in a 30-minute interview:
// ~15 lines, no recursion, half the memory, smaller constant factor.
// ============================================================================
#include <bits/stdc++.h>
using namespace std;

// ----------------------------------------------------------------------------
// 1. ITERATIVE SEGMENT TREE (bottom-up)  — the interview-optimal version
//
// Layout: array t of size 2n. Leaves live at t[n..2n-1] (t[n+i] = a[i]).
// Internal node i has children 2i and 2i+1, so parent of i is i>>1,
// and sibling of i is i^1. Root is t[1].
//
// Build : O(n)          — exactly n-1 combines
// Update: O(log n)      — exactly floor(log2 n)+1 nodes touched
// Query : O(log n)      — at most 2 combines per level, <= 2*ceil(log2 n)
// Memory: 2n            (recursive version needs 4n)
// ----------------------------------------------------------------------------
struct IterSegTree {
    int n;
    vector<long long> t;                    // 1-indexed implicit tree, size 2n

    explicit IterSegTree(const vector<long long>& a)
        : n((int)a.size()), t(2 * a.size()) {
        copy(a.begin(), a.end(), t.begin() + n);        // leaves
        for (int i = n - 1; i >= 1; --i)                // parents, bottom-up
            t[i] = t[2 * i] + t[2 * i + 1];
    }

    // Point assign a[pos] = val, then fix the O(log n) ancestors.
    void update(int pos, long long val) {
        for (t[pos += n] = val; pos > 1; pos >>= 1)
            t[pos >> 1] = t[pos] + t[pos ^ 1];          // pos^1 = sibling
    }

    // Sum over half-open range [l, r).
    // Invariant: everything strictly between pointers l and r at the current
    // level is already fully covered by nodes we've absorbed or will absorb.
    // If l is a RIGHT child (l&1), its parent covers elements < l too, so we
    // must take t[l] itself and step past it. Symmetric for r.
    long long query(int l, int r) const {
        long long res = 0;                              // identity of the op
        for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
            if (l & 1) res += t[l++];
            if (r & 1) res += t[--r];
        }
        return res;
    }
    // NOTE: sum is commutative so one accumulator is fine. For a
    // NON-commutative combine (e.g. matrix multiply, function composition),
    // keep two accumulators:  resL = resL*t[l++]  and  resR = t[--r]*resR,
    // answer = resL*resR. Everything else is unchanged.
};

// ----------------------------------------------------------------------------
// 2. RECURSIVE SEGMENT TREE (top-down) — same asymptotics, more flexibility
//
// Node v covers [lo, hi]; children v*2 (left half), v*2+1 (right half).
// Needs 4n memory because n is not necessarily a power of two, so the
// implicit heap layout can have index gaps.
//
// Build : O(n), Update/Query: O(log n) — a query recurses into at most
// 4 nodes per level (proof: at each level only the two "border" nodes can
// be partially covered; fully-covered nodes return immediately).
// ----------------------------------------------------------------------------
struct RecSegTree {
    int n;
    vector<long long> t;

    explicit RecSegTree(const vector<long long>& a) : n((int)a.size()), t(4 * a.size()) {
        build(a, 1, 0, n - 1);
    }
    void build(const vector<long long>& a, int v, int lo, int hi) {
        if (lo == hi) { t[v] = a[lo]; return; }
        int mid = (lo + hi) / 2;
        build(a, 2 * v, lo, mid);
        build(a, 2 * v + 1, mid + 1, hi);
        t[v] = t[2 * v] + t[2 * v + 1];
    }
    void update(int pos, long long val) { update(1, 0, n - 1, pos, val); }
    void update(int v, int lo, int hi, int pos, long long val) {
        if (lo == hi) { t[v] = val; return; }
        int mid = (lo + hi) / 2;
        if (pos <= mid) update(2 * v, lo, mid, pos, val);
        else            update(2 * v + 1, mid + 1, hi, pos, val);
        t[v] = t[2 * v] + t[2 * v + 1];
    }
    long long query(int l, int r) const { return query(1, 0, n - 1, l, r); }  // [l, r]
    long long query(int v, int lo, int hi, int l, int r) const {
        if (r < lo || hi < l) return 0;                 // disjoint
        if (l <= lo && hi <= r) return t[v];            // fully covered
        int mid = (lo + hi) / 2;
        return query(2 * v, lo, mid, l, r) + query(2 * v + 1, mid + 1, hi, l, r);
    }
};

// ----------------------------------------------------------------------------
// Demo + stress test + micro-benchmark
// ----------------------------------------------------------------------------
int main() {
    // --- Example 1: basic usage --------------------------------------------
    vector<long long> a = {5, 8, 6, 3, 2, 7, 2, 6};
    IterSegTree it(a);
    RecSegTree  rc(a);
    cout << "a = {5,8,6,3,2,7,2,6}\n";
    cout << "sum[2,5]  iterative=" << it.query(2, 6)   // [2,6) == indices 2..5
         << "  recursive=" << rc.query(2, 5) << "  expected=18\n";
    it.update(3, 10); rc.update(3, 10);                 // a[3]: 3 -> 10
    cout << "after a[3]=10, sum[2,5]  iterative=" << it.query(2, 6)
         << "  recursive=" << rc.query(2, 5) << "  expected=25\n";

    // --- Example 2: n NOT a power of two (the classic pitfall) -------------
    vector<long long> b = {1, 2, 3, 4, 5};              // n = 5
    IterSegTree itb(b);
    cout << "n=5, sum[1,3] = " << itb.query(1, 4) << "  expected=9\n";

    // --- Stress test: both trees vs brute force ----------------------------
    mt19937 rng(42);
    int N = 237;                                        // deliberately odd size
    vector<long long> arr(N);
    for (auto& x : arr) x = (long long)(rng() % 2001) - 1000;
    IterSegTree st1(arr);
    RecSegTree  st2(arr);
    for (int iter = 0; iter < 20000; ++iter) {
        if (rng() % 2) {                                // random point update
            int p = rng() % N; long long v = (long long)(rng() % 2001) - 1000;
            arr[p] = v; st1.update(p, v); st2.update(p, v);
        } else {                                        // random range query
            int l = rng() % N, r = rng() % N;
            if (l > r) swap(l, r);
            long long brute = accumulate(arr.begin() + l, arr.begin() + r + 1, 0LL);
            assert(st1.query(l, r + 1) == brute);
            assert(st2.query(l, r) == brute);
        }
    }
    cout << "stress test: 20000 mixed ops on n=237 -> all three agree ✓\n";

    // --- Micro-benchmark: constant-factor difference ------------------------
    int M = 1 << 20;                                    // n = 1,048,576
    vector<long long> big(M);
    for (auto& x : big) x = rng() % 1000;
    IterSegTree bi(big);
    RecSegTree  br(big);
    int Q = 1'000'000;
    vector<pair<int,int>> qs(Q);
    for (auto& [l, r] : qs) { l = rng() % M; r = rng() % M; if (l > r) swap(l, r); }

    volatile long long sink = 0;
    auto t0 = chrono::steady_clock::now();
    for (auto& [l, r] : qs) sink += bi.query(l, r + 1);
    auto t1 = chrono::steady_clock::now();
    for (auto& [l, r] : qs) sink += br.query(l, r);
    auto t2 = chrono::steady_clock::now();
    auto ms = [](auto d) { return chrono::duration_cast<chrono::milliseconds>(d).count(); };
    cout << "1e6 random queries, n=2^20:  iterative " << ms(t1 - t0)
         << " ms   vs   recursive " << ms(t2 - t1) << " ms\n";
    return 0;
}
