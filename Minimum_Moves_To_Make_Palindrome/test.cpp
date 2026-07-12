// Tests for Minimum Moves to Make Palindrome.
//
// 1. Fixed known cases.
// 2. Brute force: BFS over the swap graph (each state = string, each edge =
//    one adjacent swap) gives the TRUE minimum for small n. Cross-check the
//    greedy against it on thousands of random palindromable strings.
//
// Build:  g++ -O2 -std=c++17 -o test test.cpp && ./test

#include <bits/stdc++.h>
using namespace std;

#include "solution.cpp"
#include "solution_nlogn.cpp"
#include "solution_segtree.cpp"
#include "solution_mergesort.cpp"

// ---------- brute force: exact shortest path in the swap graph ----------
static int bruteForce(const string& start) {
    auto isPal = [](const string& t) {
        for (int i = 0, j = (int)t.size() - 1; i < j; ++i, --j)
            if (t[i] != t[j]) return false;
        return true;
    };
    if (isPal(start)) return 0;
    queue<pair<string, int>> q;
    unordered_set<string> seen{start};
    q.push({start, 0});
    while (!q.empty()) {
        auto [cur, d] = q.front(); q.pop();
        for (int i = 0; i + 1 < (int)cur.size(); ++i) {
            string nxt = cur;
            swap(nxt[i], nxt[i + 1]);
            if (!seen.insert(nxt).second) continue;
            if (isPal(nxt)) return d + 1;
            q.push({nxt, d + 1});
        }
    }
    return -1; // unreachable: input guaranteed palindromable
}

// random palindromable string: build a palindrome, then shuffle
// (shuffling preserves the multiset, hence palindromability)
static string randomPalindromable(mt19937& rng, int n, int alpha) {
    string t(n, '?');
    for (int i = 0, j = n - 1; i <= j; ++i, --j)
        t[i] = t[j] = char('a' + rng() % alpha);
    shuffle(t.begin(), t.end(), rng);
    return t;
}

int main() {
    int passed = 0;
    auto expect = [&](const string& s, int want, const string& label) {
        int got = minMovesToMakePalindrome(s);
        if (got != want) {
            cout << "FAIL [" << label << "] s=\"" << s << "\" got " << got
                 << " want " << want << '\n';
            exit(1);
        }
        ++passed;
    };

    // ---- fixed cases ----
    expect("aabb",   2, "LC example 1");
    expect("letelt", 2, "LC example 2");
    expect("a",      0, "single char");
    expect("aa",     0, "already palindrome");
    expect("racecar",0, "already palindrome, odd");
    expect("aaaa",   0, "all equal");
    expect("cbaab",  bruteForce("cbaab"), "odd letter at s[0]");
    expect("eelle",  bruteForce("eelle"), "odd defer mid-run");

    // ---- randomized cross-check vs exact BFS ----
    mt19937 rng(20260710);
    for (int iter = 0; iter < 3000; ++iter) {
        int n     = 2 + (int)(rng() % 7);          // lengths 2..8
        int alpha = 1 + (int)(rng() % 3);          // alphabets a / ab / abc
        string s  = randomPalindromable(rng, n, alpha);
        int want  = bruteForce(s);
        int got   = minMovesToMakePalindrome(s);
        long long fast = minMovesToMakePalindromeFast(s);
        long long seg  = minMovesToMakePalindromeSeg(s);
        long long segR = minMovesToMakePalindromeSegRec(s);
        long long mrg  = minMovesToMakePalindromeMerge(s);
        if (got != want || fast != want || seg != want || segR != want || mrg != want) {
            cout << "FAIL [random] s=\"" << s << "\" greedy=" << got
                 << " nlogn=" << fast << " segtree=" << seg
                 << " segtree-rec=" << segR << " mergesort=" << mrg
                 << " want " << want << '\n';
            return 1;
        }
        ++passed;
    }

    // ---- O(n log n) vs O(n^2) greedy on larger random strings ----
    {
        mt19937 r3(42);
        for (int iter = 0; iter < 400; ++iter) {
            int n     = 2 + (int)(r3() % 300);
            int alpha = 1 + (int)(r3() % 6);
            string s  = randomPalindromable(r3, n, alpha);
            int a = minMovesToMakePalindrome(s);
            long long b = minMovesToMakePalindromeFast(s);
            long long c = minMovesToMakePalindromeSeg(s);
            long long d = minMovesToMakePalindromeSegRec(s);
            long long e = minMovesToMakePalindromeMerge(s);
            if (a != b || a != c || a != d || a != e) {
                cout << "FAIL [fast-vs-greedy] n=" << n << " s=\"" << s
                     << "\" greedy=" << a << " nlogn=" << b
                     << " segtree=" << c << " segtree-rec=" << d
                     << " mergesort=" << e << '\n';
                return 1;
            }
            ++passed;
        }
    }

    // ---- performance guard: worst-ish case at max n ----
    {
        mt19937 r2(7);
        string big = randomPalindromable(r2, 2000, 2);
        auto t0 = chrono::steady_clock::now();
        volatile int ans = minMovesToMakePalindrome(big);
        (void)ans;
        auto ms = chrono::duration_cast<chrono::milliseconds>(
                      chrono::steady_clock::now() - t0).count();
        cout << "n=2000 ran in " << ms << " ms\n";
        ++passed;
    }

    cout << "ALL " << passed << " CHECKS PASSED\n";
    return 0;
}
