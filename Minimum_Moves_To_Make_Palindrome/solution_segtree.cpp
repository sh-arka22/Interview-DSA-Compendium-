// O(n log n) version with a SEGMENT TREE (simplest form) instead of a BIT.
//
// Same reduction as solution_nlogn.cpp:
//   (1) build the target permutation (pair k-th occurrence with (count-k+1)-th,
//       shells ordered by left endpoint, odd letter's middle -> n/2)   O(n)
//   (2) answer = inversions of target[]                                O(n log n)
//
// Here (2) uses the simplest segment tree there is:
//
//   - We need only two operations over an array cnt[0..n-1] (initially 0):
//       add(i)      : cnt[i] += 1                      (point update)
//       query(l, r) : sum of cnt[l..r-1]               (range sum)
//   - Iterative, bottom-up, array of size 2n. Leaf i lives at t[n + i];
//     internal node p = sum of children t[2p] and t[2p+1]. No build, no
//     recursion, no lazy propagation — ~10 lines total.
//
//   layout for n = 4:            t[1]
//                              /      \
//                          t[2]        t[3]
//                          /  \        /  \
//                       t[4] t[5]  t[6]  t[7]
//                       cnt0 cnt1  cnt2  cnt3   <- leaves = t[n..2n-1]
//
// Inversion sweep: scan i from right to left; leaves already added are
// exactly the elements to the RIGHT of i, so query(0, target[i]) counts
// how many of them are smaller -> inversions contributed by i.
//
// Complexity: O(n log n) time, O(n) space.

#include <bits/stdc++.h>
using namespace std;

struct SegTree {                       // point add, range-sum, sums of +1s
    int n;
    vector<int> t;                     // t[1..2n-1]; leaves at t[n+i]
    SegTree(int n) : n(n), t(2 * n, 0) {}

    void add(int i) {                  // cnt[i] += 1
        for (t[i += n] += 1; i > 1; i >>= 1)
            t[i >> 1] = t[i] + t[i ^ 1];   // parent = child + sibling
    }
    int query(int l, int r) {          // sum of cnt[l..r-1], 0 if l >= r
        int s = 0;
        for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
            if (l & 1) s += t[l++];    // l is a right child: take it, step past
            if (r & 1) s += t[--r];    // r is a right child: take its left sibling
        }
        return s;
    }
};

// ---------------------------------------------------------------------------
// Variant 2: RECURSIVE segment tree (classic textbook top-down form).
//
// Same two operations (point add, range sum), but every node explicitly owns
// a segment [lo, hi] and delegates to its two halves. This is the form that
// generalises to lazy propagation, segment-tree-on-ranges, merge trees, etc.
//
//   node p  owns [lo, hi]
//   left  child 2p   owns [lo, mid]
//   right child 2p+1 owns [mid+1, hi],   mid = (lo + hi) / 2
//
//   add(i):      walk down the ONE path whose segment contains i,
//                +1 at every node on the way.                    O(log n)
//   query(l,r):  three cases at each node —
//                  disjoint   -> contribute 0
//                  contained  -> contribute t[p] (stop descending)
//                  partial    -> recurse into both children       O(log n)
//
// Memory: 4n nodes is the safe bound for this indexing scheme.
// ---------------------------------------------------------------------------

struct SegTreeRec {
    int n;
    vector<int> t;                             // t[p] = sum over p's segment
    SegTreeRec(int n) : n(n), t(4 * n, 0) {}

    void add(int i) { add(1, 0, n - 1, i); }
    int  query(int l, int r) {                 // sum of cnt[l..r], 0 if l > r
        if (l > r) return 0;
        return query(1, 0, n - 1, l, r);
    }

private:
    void add(int p, int lo, int hi, int i) {
        ++t[p];                                // i is inside [lo, hi] by invariant
        if (lo == hi) return;                  // leaf reached
        int mid = (lo + hi) / 2;
        if (i <= mid) add(2 * p,     lo,      mid, i);
        else          add(2 * p + 1, mid + 1, hi,  i);
    }
    int query(int p, int lo, int hi, int l, int r) {
        if (r < lo || hi < l) return 0;        // disjoint
        if (l <= lo && hi <= r) return t[p];   // fully contained
        int mid = (lo + hi) / 2;               // partial overlap: split
        return query(2 * p,     lo,      mid, l, r)
             + query(2 * p + 1, mid + 1, hi,  l, r);
    }
};

long long minMovesToMakePalindromeSegRec(const string& s) {
    int n = (int)s.size();

    // ---- (1) target permutation (identical construction) ----
    vector<vector<int>> pos(26);
    for (int i = 0; i < n; ++i) pos[s[i] - 'a'].push_back(i);

    vector<pair<int, int>> pairs;
    int center = -1;
    for (auto& v : pos) {
        int m = (int)v.size();
        for (int k = 0; k < m / 2; ++k) pairs.push_back({v[k], v[m - 1 - k]});
        if (m & 1) center = v[m / 2];
    }
    sort(pairs.begin(), pairs.end());

    vector<int> target(n);
    for (int k = 0; k < (int)pairs.size(); ++k) {
        target[pairs[k].first]  = k;
        target[pairs[k].second] = n - 1 - k;
    }
    if (center != -1) target[center] = n / 2;

    // ---- (2) inversion count, recursive tree, inclusive query [0, x-1] ----
    SegTreeRec st(n);
    long long inv = 0;
    for (int i = n - 1; i >= 0; --i) {
        inv += st.query(0, target[i] - 1);
        st.add(target[i]);
    }
    return inv;
}

long long minMovesToMakePalindromeSeg(const string& s) {
    int n = (int)s.size();

    // ---- (1) target permutation (identical to solution_nlogn.cpp) ----
    vector<vector<int>> pos(26);
    for (int i = 0; i < n; ++i) pos[s[i] - 'a'].push_back(i);

    vector<pair<int, int>> pairs;
    int center = -1;
    for (auto& v : pos) {
        int m = (int)v.size();
        for (int k = 0; k < m / 2; ++k) pairs.push_back({v[k], v[m - 1 - k]});
        if (m & 1) center = v[m / 2];
    }
    sort(pairs.begin(), pairs.end());

    vector<int> target(n);
    for (int k = 0; k < (int)pairs.size(); ++k) {
        target[pairs[k].first]  = k;
        target[pairs[k].second] = n - 1 - k;
    }
    if (center != -1) target[center] = n / 2;

    // ---- (2) inversion count with the segment tree ----
    SegTree st(n);
    long long inv = 0;
    for (int i = n - 1; i >= 0; --i) {
        inv += st.query(0, target[i]);     // smaller targets already to the right
        st.add(target[i]);
    }
    return inv;
}
