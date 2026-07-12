// THE STACK APPROACH — and why it fails for this problem.
//
// ---------------------------------------------------------------------------
// The instinct (and it is a good instinct elsewhere)
// ---------------------------------------------------------------------------
// Problems like Valid Parentheses / Min Remove to Make Valid train you to
// pair symbols with a per-character STACK: scan left to right; when you see
// letter c and an unmatched c is on the stack, pop -> that's a pair (LIFO,
// nearest-unmatched pairing).
//
// One pass, O(n). Feels perfect for pairing palindrome twins. It isn't.
//
// ---------------------------------------------------------------------------
// The first-principles reason it fails
// ---------------------------------------------------------------------------
// Stack matching produces bracket structure: any two pairs are either
// NESTED     ( [ ] )        or  SIDE-BY-SIDE   ( ) [ ]
// and both are legal for brackets.
//
// A palindrome is stricter. Its pairs are the shells (0,n-1),(1,n-2),... —
// EVERY pair strictly contains the next. Fully concentric. SIDE-BY-SIDE
// pairs are precisely what a palindrome cannot have:
//
//   bracket-legal:   a . a  b . b        pairs (0,2) and (3,5)  — disjoint
//   palindrome:      a  b  .  .  b  a    pairs must wrap around each other
//
// So when the LIFO stack emits two disjoint pairs of the same letter, it has
// committed to a target no palindrome matches; repairing that costs swaps
// the true optimum never pays. The correct pairing is FIFO per letter —
// k-th occurrence with (count-k+1)-th, i.e. FIRST pairs with LAST — the
// unique fully-concentric pairing (this is the never-cross lemma in action;
// see ALGORITHM.md par.1 and par.3B).
//
// ---------------------------------------------------------------------------
// The killer counterexample:  s = "abaaba"
// ---------------------------------------------------------------------------
// 'a' at 0,2,3,5.  s is ALREADY a palindrome -> true answer 0.
//
//   LIFO stack pairing of 'a':  (0,2) and (3,5)   <- side-by-side. Illegal
//        shape for a palindrome; forces a rearrangement costing 4 swaps.
//   FIFO pairing of 'a':        (0,5) and (2,3)   <- concentric. Cost 0.
//
// Moral: the data structure the instinct is reaching for is a per-letter
// DEQUE (pair front with back), not a stack. And FIFO pairing + inversion
// count is exactly solution_nlogn.cpp — so the "fixed stack approach"
// collapses into the O(n log n) solution we already have. There is no
// stack-based algorithm that beats it: counting the swaps IS counting
// inversions, and that needs a BIT / segment tree / merge sort.
//
// This file implements BOTH pairings behind one inversion counter, prints
// the counterexample, then measures LIFO's failure rate on random inputs.
//
// Build:  g++ -O2 -std=c++17 -o stack_analysis stack_approach_analysis.cpp
// ---------------------------------------------------------------------------

#include <bits/stdc++.h>
using namespace std;

#include "solution.cpp"   // verified O(n^2) greedy = ground truth

// shared back-end: pairs -> shells -> target permutation -> inversions
static long long costOfPairing(int n, vector<pair<int,int>> pairs, int center) {
    sort(pairs.begin(), pairs.end());
    vector<int> target(n);
    for (int k = 0; k < (int)pairs.size(); ++k) {
        target[pairs[k].first]  = k;
        target[pairs[k].second] = n - 1 - k;
    }
    if (center != -1) target[center] = n / 2;

    vector<int> bit(n + 1, 0);
    auto add = [&](int i) { for (++i; i <= n; i += i & -i) ++bit[i]; };
    auto pre = [&](int i) { int r = 0; for (++i; i > 0; i -= i & -i) r += bit[i]; return r; };
    long long inv = 0;
    for (int i = n - 1; i >= 0; --i) { inv += pre(target[i] - 1); add(target[i]); }
    return inv;
}

// LIFO: bracket-style, nearest-unmatched pairing (the tempting, wrong one)
static long long stackLIFO(const string& s) {
    int n = (int)s.size();
    array<vector<int>, 26> stk{};                 // per-letter stacks
    vector<pair<int,int>> pairs;
    int center = -1;
    for (int i = 0; i < n; ++i) {
        auto& st = stk[s[i] - 'a'];
        if (!st.empty()) { pairs.push_back({st.back(), i}); st.pop_back(); }
        else st.push_back(i);
    }
    for (auto& st : stk) if (!st.empty()) center = st[0];  // the odd letter
    return costOfPairing(n, move(pairs), center);
}

// FIFO: first-with-last, fully concentric (the correct one = solution_nlogn)
static long long dequeFIFO(const string& s) {
    int n = (int)s.size();
    array<vector<int>, 26> pos{};
    for (int i = 0; i < n; ++i) pos[s[i] - 'a'].push_back(i);
    vector<pair<int,int>> pairs;
    int center = -1;
    for (auto& v : pos) {
        int m = (int)v.size();
        for (int k = 0; k < m / 2; ++k) pairs.push_back({v[k], v[m - 1 - k]});
        if (m & 1) center = v[m / 2];
    }
    return costOfPairing(n, move(pairs), center);
}

int main() {
    // ---- the counterexample, spelled out ----
    string cx = "abaaba";
    cout << "s = \"" << cx << "\"  (already a palindrome)\n";
    cout << "  true optimum (greedy) : " << minMovesToMakePalindrome(cx) << '\n';
    cout << "  FIFO concentric pairs : " << dequeFIFO(cx)  << "   <- correct\n";
    cout << "  LIFO stack pairs      : " << stackLIFO(cx) << "   <- overpays: (0,2),(3,5) are side-by-side\n\n";

    // ---- how badly and how often does LIFO fail? ----
    mt19937 rng(20260710);
    int trials = 5000, wrong = 0;
    long long worstGap = 0; string worstCase;
    for (int t = 0; t < trials; ++t) {
        int n = 2 + (int)(rng() % 11), alpha = 1 + (int)(rng() % 3);
        string s(n, '?');
        for (int i = 0, j = n - 1; i <= j; ++i, --j)
            s[i] = s[j] = char('a' + rng() % alpha);
        shuffle(s.begin(), s.end(), rng);

        long long truth = minMovesToMakePalindrome(s);
        long long fifo  = dequeFIFO(s);
        long long lifo  = stackLIFO(s);
        assert(fifo == truth);                    // FIFO is always optimal
        assert(lifo >= truth);                    // LIFO never beats it
        if (lifo != truth) {
            ++wrong;
            if (lifo - truth > worstGap) { worstGap = lifo - truth; worstCase = s; }
        }
    }
    cout << "random trials            : " << trials << "  (n in [2,12])\n";
    cout << "FIFO pairing wrong       : 0  (matches greedy every time)\n";
    cout << "LIFO stack pairing wrong : " << wrong << "  ("
         << fixed << setprecision(1) << 100.0 * wrong / trials << "%)\n";
    cout << "worst overpayment        : +" << worstGap
         << "  on s=\"" << worstCase << "\"\n";
    return 0;
}
