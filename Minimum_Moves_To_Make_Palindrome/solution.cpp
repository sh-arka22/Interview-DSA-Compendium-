// Minimum Number of Moves to Make Palindrome (LeetCode 2193, Hard)
//
// Approach   : Two pointers, outermost shell first. Pair s[l] with the
//              RIGHTMOST occurrence of the same letter in (l, r] and bubble
//              it to position r (cost r - j). If no twin exists, s[l] is the
//              unique odd-count letter: defer it one step inward (cost 1)
//              and re-examine l without shrinking the window.
// Why optimal: min adjacent swaps to a fixed target = inversion count;
//              identical letters never cross in an optimal solution, which
//              forces exactly the rightmost-twin pairing. See ALGORITHM.md.
// Complexity : O(n^2) time, O(n) space. n <= 2000 -> ~4e6 ops, instant.

#include <bits/stdc++.h>
using namespace std;

int minMovesToMakePalindrome(string s) {
    int ans = 0;
    int l = 0, r = (int)s.size() - 1;
    while (l < r) {
        int j = r;
        while (j > l && s[j] != s[l]) --j;   // rightmost twin of s[l] in (l, r]

        if (j == l) {                        // odd-count letter: defer inward
            swap(s[l], s[l + 1]);
            ++ans;                           // window NOT shrunk; retry l
        } else {
            for (int k = j; k < r; ++k) {    // bubble twin to the right end
                swap(s[k], s[k + 1]);
                ++ans;
            }
            ++l;                             // shell fixed, peel inward
            --r;
        }
    }
    return ans;
}


int minMovesToMakePalindrome(string s) {
    int n = s.size();
    array<deque<int>, 26> pos;
    for (int i = 0; i < n; ++i) pos[s[i] - 'a'].push_back(i);

    vector<int> target(n);
    int l = 0;
    for (int i = 0; i < n; ++i) {
        auto& d = pos[s[i] - 'a'];
        if (d.empty() || d.front() != i) continue;        // i already used as a right twin
        if (d.size() == 1) {                              // odd letter → middle
            target[i] = n / 2;
            d.pop_front();
        } else {                                          // shell l: (leftmost, rightmost twin)
            target[i] = l;
            target[d.back()] = n - 1 - l;
            d.pop_front(); d.pop_back();
            ++l;
        }
    }

    // min adjacent swaps = inversions of target[]
    vector<int> bit(n + 1, 0);
    auto upd = [&](int i) { for (++i; i <= n; i += i & -i) ++bit[i]; };
    auto qry = [&](int i) { int r = 0; for (++i; i > 0; i -= i & -i) r += bit[i]; return r; };

    long long ans = 0;
    for (int i = n - 1; i >= 0; --i) {                    // count j > i with target[j] < target[i]
        ans += qry(target[i] - 1);
        upd(target[i]);
    }
    return (int)ans;
}

// LeetCode wrapper
class Solution {
public:
    int minMovesToMakePalindrome(string s) { return ::minMovesToMakePalindrome(move(s)); }
};

#ifdef LOCAL_DEMO
int main() {
    cout << minMovesToMakePalindrome("aabb")   << '\n';  // 2
    cout << minMovesToMakePalindrome("letelt") << '\n';  // 2
    cout << minMovesToMakePalindrome("cbaab")  << '\n';  // 2
}
#endif
