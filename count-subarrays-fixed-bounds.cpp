// LeetCode 2444 — Count Subarrays With Fixed Bounds (Hard)
// ============================================================
// INTERVIEW SOLUTION (30-min optimal): Approach 3 — three-pointer O(n)/O(1).
// All other approaches included below for study + stress test in main().
//
// Core reframe: fix the RIGHT endpoint R, count valid LEFT endpoints L.
// Subarray [L..R] is fixed-bound iff:
//   (a) no element in [L..R] is outside [minK, maxK]  -> L > lastBad
//   (b) minK appears in [L..R]                        -> L <= lastMin
//   (c) maxK appears in [L..R]                        -> L <= lastMax
// => count for this R = max(0, min(lastMin, lastMax) - lastBad)

#include <bits/stdc++.h>
using namespace std;

// ------------------------------------------------------------
// Approach 1 — Brute force with running min/max. O(n^2) / O(1).
// Fine for n <= 10^3 (this statement), TLEs at LeetCode's n = 10^5.
// ------------------------------------------------------------
long long solveBrute(const vector<int>& a, int minK, int maxK) {
    int n = a.size();
    long long ans = 0;
    for (int l = 0; l < n; ++l) {
        int mn = INT_MAX, mx = INT_MIN;
        for (int r = l; r < n; ++r) {
            mn = min(mn, a[r]);
            mx = max(mx, a[r]);
            if (mn < minK || mx > maxK) break;   // pruning: window is dead forever
            if (mn == minK && mx == maxK) ++ans;
        }
    }
    return ans;
}

// ------------------------------------------------------------
// Approach 2 — Segment split + inclusion–exclusion. O(n) / O(1).
// Split array at "bad" elements (outside [minK,maxK]). In each clean
// segment of length m:
//   valid = total - (missing minK) - (missing maxK) + (missing both)
// where "missing X" subarrays are counted by summing maximal runs
// free of X: run of length k contributes k*(k+1)/2.
// ------------------------------------------------------------
long long solveInclExcl(const vector<int>& a, int minK, int maxK) {
    int n = a.size();
    auto runs = [&](int lo, int hi, auto skip) -> long long { // subarrays of a[lo..hi] avoiding skip(x)
        long long s = 0, len = 0;
        for (int i = lo; i <= hi; ++i) {
            len = skip(a[i]) ? 0 : len + 1;
            s += len;
        }
        return s;
    };
    long long ans = 0;
    int i = 0;
    while (i < n) {
        if (a[i] < minK || a[i] > maxK) { ++i; continue; }
        int j = i;
        while (j + 1 < n && a[j + 1] >= minK && a[j + 1] <= maxK) ++j;
        long long m = j - i + 1;
        long long total   = m * (m + 1) / 2;
        long long noMin   = runs(i, j, [&](int x){ return x == minK; });
        long long noMax   = runs(i, j, [&](int x){ return x == maxK; });
        long long noBoth  = runs(i, j, [&](int x){ return x == minK || x == maxK; });
        ans += total - noMin - noMax + noBoth;
        i = j + 1;
    }
    return ans;
}

// ------------------------------------------------------------
// Approach 3 — THE INTERVIEW ANSWER. Three pointers, one pass.
// O(n) time, O(1) space, ~7 lines.
// ------------------------------------------------------------
long long countSubarrays(const vector<int>& nums, int minK, int maxK) {
    long long ans = 0;
    long long lastMin = -1, lastMax = -1, lastBad = -1;
    for (long long i = 0; i < (long long)nums.size(); ++i) {
        if (nums[i] < minK || nums[i] > maxK) lastBad = i;
        if (nums[i] == minK) lastMin = i;
        if (nums[i] == maxK) lastMax = i;
        ans += max(0LL, min(lastMin, lastMax) - lastBad);
    }
    return ans;
}

// ------------------------------------------------------------
// Approach 4 — Sparse table RMQ + binary search per left endpoint.
// O(n log n) / O(n log n). For fixed L, min(L..R) and max(L..R) are
// monotone in R, so the set of valid R is a contiguous interval found
// by 4 binary searches. Generalizes to arbitrary (min,max) predicates.
// ------------------------------------------------------------
struct SparseTable {
    int n, LOG; vector<vector<int>> mn, mx;
    SparseTable(const vector<int>& a) : n(a.size()) {
        LOG = 32 - __builtin_clz(max(n, 1));
        mn.assign(LOG, vector<int>(n)); mx = mn;
        mn[0] = a; mx[0] = a;
        for (int k = 1; k < LOG; ++k)
            for (int i = 0; i + (1 << k) <= n; ++i) {
                mn[k][i] = min(mn[k-1][i], mn[k-1][i + (1 << (k-1))]);
                mx[k][i] = max(mx[k-1][i], mx[k-1][i + (1 << (k-1))]);
            }
    }
    int qmin(int l, int r) const { int k = 31 - __builtin_clz(r - l + 1); return min(mn[k][l], mn[k][r - (1 << k) + 1]); }
    int qmax(int l, int r) const { int k = 31 - __builtin_clz(r - l + 1); return max(mx[k][l], mx[k][r - (1 << k) + 1]); }
};

long long solveRMQ(const vector<int>& a, int minK, int maxK) {
    int n = a.size();
    if (n == 0) return 0;
    SparseTable st(a);
    long long ans = 0;
    for (int l = 0; l < n; ++l) {
        // valid R interval: min hits minK before dropping below; max hits maxK before rising above
        auto firstTrue = [&](auto pred) {   // smallest r in [l, n-1] with pred(r), else n
            int lo = l, hi = n;
            while (lo < hi) { int mid = (lo + hi) / 2; pred(mid) ? hi = mid : lo = mid + 1; }
            return lo;
        };
        int rMinStart = firstTrue([&](int r){ return st.qmin(l, r) <= minK; });     // min first reaches <= minK
        int rMinEnd   = firstTrue([&](int r){ return st.qmin(l, r) <  minK; }) - 1; // before dropping below
        int rMaxStart = firstTrue([&](int r){ return st.qmax(l, r) >= maxK; });
        int rMaxEnd   = firstTrue([&](int r){ return st.qmax(l, r) >  maxK; }) - 1;
        int lo = max(rMinStart, rMaxStart), hi = min({rMinEnd, rMaxEnd, n - 1});
        // check the min actually EQUALS minK on this interval (not just <=)
        if (lo <= hi && st.qmin(l, lo) == minK && st.qmax(l, lo) == maxK)
            ans += hi - lo + 1;
    }
    return ans;
}

// ------------------------------------------------------------
// NOVEL IDEA 1 — Left-endpoint state automaton. O(n) / O(1).
// Classify every ACTIVE left endpoint L by a 2-bit state of [L..R]:
//   bit0 = contains minK, bit1 = contains maxK.
// Keep only the 4 COUNTS c[0..3]. New element x:
//   out of range -> all counts reset (every window dies)
//   x == minK    -> everything gains bit0: c[1]+=c[0], c[3]+=c[2], c[0]=c[2]=0
//   x == maxK    -> everything gains bit1: c[2]+=c[0], c[3]+=c[1], c[0]=c[1]=0
//   then L = R joins as a fresh window with its own state.
// Answer accumulates c[3]. No index bookkeeping at all — the three
// pointers of Approach 3 fall out as boundaries between state classes.
// Generalizes to k required values (2^k counters) where the
// three-pointer trick does not.
// ------------------------------------------------------------
long long solveAutomaton(const vector<int>& a, int minK, int maxK) {
    long long c[4] = {0, 0, 0, 0}, ans = 0;
    for (int x : a) {
        if (x < minK || x > maxK) { c[0] = c[1] = c[2] = c[3] = 0; continue; }
        if (x == minK) { c[1] += c[0]; c[3] += c[2]; c[0] = c[2] = 0; }
        if (x == maxK) { c[2] += c[0]; c[3] += c[1]; c[0] = c[1] = 0; }
        ++c[(x == minK ? 1 : 0) | (x == maxK ? 2 : 0)];
        ans += c[3];
    }
    return ans;
}

// ------------------------------------------------------------
// NOVEL IDEA 2 — Bitset word-parallel counting. O(n^2 / 64) / O(n).
// seenMin bit L == 1  <=>  [L..R] contains minK (and no bad element).
// When x == minK every active left endpoint acquires the flag at once:
// seenMin |= activeMask — a word-parallel "broadcast". Answer for this
// R is popcount(seenMin & seenMax). Beats O(n^2) by 64x; at n = 10^3
// that's ~16k word ops total. Pattern worth knowing: many O(n^2)
// subarray-counting DPs become O(n^2/64) when the per-L state is boolean.
// ------------------------------------------------------------
long long solveBitset(const vector<int>& a, int minK, int maxK) {
    const int n = a.size(), W = (n + 63) / 64;
    vector<uint64_t> seenMin(W, 0), seenMax(W, 0), active(W, 0);
    long long ans = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < minK || a[i] > maxK) {
            fill(active.begin(), active.end(), 0ULL);
            fill(seenMin.begin(), seenMin.end(), 0ULL);
            fill(seenMax.begin(), seenMax.end(), 0ULL);
            continue;
        }
        active[i >> 6] |= 1ULL << (i & 63);
        if (a[i] == minK) for (int w = 0; w < W; ++w) seenMin[w] = active[w];
        if (a[i] == maxK) for (int w = 0; w < W; ++w) seenMax[w] = active[w];
        for (int w = 0; w < W; ++w) ans += __builtin_popcountll(seenMin[w] & seenMax[w]);
    }
    return ans;
}

// ------------------------------------------------------------
// Stress test: all 5 implementations must agree on random inputs.
// ------------------------------------------------------------
int main() {
    mt19937 rng(42);
    for (int iter = 0; iter < 20000; ++iter) {
        int n = 2 + rng() % 12, V = 1 + rng() % 5;
        vector<int> a(n);
        for (int& x : a) x = 1 + rng() % V;
        int minK = 1 + rng() % V, maxK = 1 + rng() % V;
        long long r1 = solveBrute(a, minK, maxK);
        long long r2 = solveInclExcl(a, minK, maxK);
        long long r3 = countSubarrays(a, minK, maxK);
        long long r4 = solveRMQ(a, minK, maxK);
        long long r5 = solveAutomaton(a, minK, maxK);
        long long r6 = solveBitset(a, minK, maxK);
        if (!(r1 == r2 && r2 == r3 && r3 == r4 && r4 == r5 && r5 == r6)) {
            printf("MISMATCH minK=%d maxK=%d [", minK, maxK);
            for (int x : a) printf("%d ", x);
            printf("] brute=%lld ie=%lld 3ptr=%lld rmq=%lld auto=%lld bitset=%lld\n", r1, r2, r3, r4, r5, r6);
            return 1;
        }
    }
    // LeetCode samples
    vector<int> e1 = {1,3,5,2,7,5}, e2 = {1,1,1,1};
    printf("example1 = %lld (expect 2)\n", countSubarrays(e1, 1, 5));
    printf("example2 = %lld (expect 10)\n", countSubarrays(e2, 1, 1));
    puts("ALL STRESS TESTS PASSED");
    return 0;
}
