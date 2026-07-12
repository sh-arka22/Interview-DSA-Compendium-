// O(n log n) version — same greedy pairing, BIT inversion counting.
//
// Insight: the O(n^2) greedy does two separable jobs:
//   (1) DECIDE the pairing/target: for each letter, its k-th occurrence pairs
//       with its (count-k+1)-th occurrence (never-cross lemma); the pair with
//       the k-th smallest LEFT endpoint occupies shell k, i.e. target
//       positions (k, n-1-k); the odd letter's middle occurrence -> n/2.
//   (2) PAY for it: number of adjacent swaps = inversions of the target
//       permutation read in original string order.
// (1) is O(n). (2) is inversion counting -> Fenwick tree, O(n log n).
// The physical swapping in solution.cpp was only ever doing (2) the slow way.
//
// Complexity: O(n log n) time, O(n) space.

#include <bits/stdc++.h>
using namespace std;

long long minMovesToMakePalindromeFast(const string& s) {
    int n = (int)s.size();

    // ---- (1) build target permutation ----
    vector<vector<int>> pos(26);
    for (int i = 0; i < n; ++i) pos[s[i] - 'a'].push_back(i);

    vector<pair<int, int>> pairs;          // (left index, right index)
    int center = -1;
    for (auto& v : pos) {
        int m = (int)v.size();
        for (int k = 0; k < m / 2; ++k) pairs.push_back({v[k], v[m - 1 - k]});
        if (m & 1) center = v[m / 2];      // at most one odd letter
    }
    sort(pairs.begin(), pairs.end());      // shells ordered by left endpoint

    vector<int> target(n);
    for (int k = 0; k < (int)pairs.size(); ++k) {
        target[pairs[k].first]  = k;           // left of shell k
        target[pairs[k].second] = n - 1 - k;   // its mirror
    }
    if (center != -1) target[center] = n / 2;

    // ---- (2) count inversions of target[] with a BIT ----
    vector<int> bit(n + 1, 0);
    auto add = [&](int i) { for (++i; i <= n; i += i & -i) ++bit[i]; };
    auto pre = [&](int i) { int r = 0; for (++i; i > 0; i -= i & -i) r += bit[i]; return r; };

    long long inv = 0;
    for (int i = n - 1; i >= 0; --i) {     // pairs (i, j), i<j, target[i] > target[j]
        inv += pre(target[i] - 1);
        add(target[i]);
    }
    return inv;
}
