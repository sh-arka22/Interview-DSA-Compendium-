// O(n log n) version — inversion counting via MERGE SORT (no data structure).
//
// Same reduction as always:
//   (1) build target permutation: k-th occurrence of each letter pairs with
//       its (count-k+1)-th; pair with k-th smallest left endpoint -> shell k,
//       i.e. targets (k, n-1-k); odd letter's middle occurrence -> n/2.  O(n)
//   (2) answer = inversions of target[].
//
// Here (2) uses merge sort's divide & conquer instead of a BIT/segment tree:
//
//   inv(A) = inv(left half) + inv(right half) + cross-inversions,
//
// and the CROSS inversions are exactly what the merge step can count for
// free: when merging two SORTED halves, the moment we take element R[j]
// before the remaining L[i..], every one of those remaining left elements
// is > R[j] and sits before it in the original order — so each such moment
// contributes (elements left in L) inversions.
//
//        L = [2 5 7]   R = [1 3 6]
//        take 1 from R  -> jumps over 2,5,7  -> +3 inversions
//        take 2 from L  -> +0
//        take 3 from R  -> jumps over 5,7    -> +2
//        take 5, 6 (+1), 7 ...               total cross = 6
//
// Every inversion is counted exactly once: at the unique recursion level
// where its two indices first fall into different halves.
//
// Recurrence: T(n) = 2T(n/2) + O(n) = O(n log n).  Space O(n) (merge buffer).
// Compared to the BIT: same asymptotics, no tree, sequential memory access —
// the classic offline way to count inversions when there are no updates.

#include <bits/stdc++.h>
using namespace std;

// counts inversions of a[lo..hi) while sorting it; buf = scratch space
static long long sortCount(vector<int>& a, vector<int>& buf, int lo, int hi) {
    if (hi - lo <= 1) return 0;
    int mid = (lo + hi) / 2;
    long long inv = sortCount(a, buf, lo, mid)     // inversions inside left
                  + sortCount(a, buf, mid, hi);    // inversions inside right

    // merge step: count cross-inversions (left element > right element)
    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        if (a[i] <= a[j]) buf[k++] = a[i++];
        else {                                     // a[j] jumps the rest of L
            inv += mid - i;
            buf[k++] = a[j++];
        }
    }
    while (i < mid) buf[k++] = a[i++];
    while (j < hi)  buf[k++] = a[j++];
    copy(buf.begin() + lo, buf.begin() + hi, a.begin() + lo);
    return inv;
}

long long minMovesToMakePalindromeMerge(const string& s) {
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

    // ---- (2) inversions via merge sort ----
    vector<int> buf(n);
    return sortCount(target, buf, 0, n);
}
