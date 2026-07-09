#include <vector>
#include <string>
#include <algorithm>
using namespace std;

class Solution {
public:
    int solution(vector<string> words) {
        // Step 1: discard words with an internal repeated character (never usable),
        //         encode every remaining word as a 26-bit char bitmask.
        vector<int> masks;
        masks.reserve(words.size());
        for (const string& w : words) {
            int m = 0;
            bool ok = true;
            for (char c : w) {
                int bit = 1 << (c - 'a');
                if (m & bit) { ok = false; break; }   // internal duplicate
                m |= bit;
            }
            if (ok) masks.push_back(m);
        }

        int n = masks.size();
        if (n == 0) return 0;

        // Try longer words first: a strong candidate found early makes the
        // upper-bound prune below cut far more of the tree.
        sort(masks.begin(), masks.end(), [](int a, int b) {
            return __builtin_popcount(a) > __builtin_popcount(b);
        });

        // Precompute each word's length once (avoid repeated popcount calls
        // in the hot recursive path).
        vector<int> pc(n);
        for (int i = 0; i < n; i++) pc[i] = __builtin_popcount(masks[i]);

        // suffixSum[i] = pc[i] + pc[i+1] + ... + pc[n-1]: a loose but valid
        // upper bound on how much MORE length could possibly be added from
        // words[i..n-1] onward (the best case where none of them overlap
        // anything). Used to prune branches that can't beat the current best.
        vector<int> suffixSum(n + 1, 0);
        for (int i = n - 1; i >= 0; i--) suffixSum[i] = suffixSum[i + 1] + pc[i];

        int best = 0;

        auto dfs = [&](auto&& self, int i, int usedMask, int len) -> void {
            // Hard cap: only 26 letters exist, so 26 is the global optimum.
            // Once reached, nothing can improve on it — stop exploring entirely.
            if (best == 26) return;

            // Branch-and-bound prune: even if every remaining word were free
            // to take (upper bound), we couldn't beat `best`, so this whole
            // subtree is dead. This also naturally covers the i == n base case
            // (suffixSum[n] == 0), so update-and-return handles both.
            if (i == n || len + suffixSum[i] <= best) {
                if (len > best) best = len;
                return;
            }

            // Take word i first (sorted longest-first -> finds strong
            // candidates fast, which strengthens the prune above sooner).
            if (!(usedMask & masks[i])) {
                self(self, i + 1, usedMask | masks[i], len + pc[i]);
            }
            self(self, i + 1, usedMask, len);   // skip word i
        };

        dfs(dfs, 0, 0, 0);
        return best;
    }
};
