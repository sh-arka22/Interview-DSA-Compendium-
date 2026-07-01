#include <vector>
#include <string>
using namespace std;

class Solution {
public:
    int solution(vector<string>& words) {
        // Step 1: filter words that have internal duplicates (unusable),
        //         represent each valid word as a 26-bit char bitmask.
        vector<int> masks;
        for (const string& w : words) {
            int m = 0;
            for (char c : w) m |= (1 << (c - 'a'));
            if (__builtin_popcount(m) == (int)w.size())  // all chars unique
                masks.push_back(m);
        }

        // Step 2: enumerate all 2^n subsets of valid words.
        // A subset is valid iff no character appears in two different words,
        // i.e. OR of all masks has the same popcount as the sum of individual ones.
        int n = masks.size(), ans = 0;
        auto solve = [&](auto self, int i, int combined, int len) {
            if (i == n) {
                ans = max(ans, len);
                return;
            }
            self(self, i + 1, combined, len);              // skip word i
            if (!(combined & masks[i]))               // ← pruning!
                self(self, i + 1, combined | masks[i],     // take word i
                    len + __builtin_popcount(masks[i]));
        };
        solve(solve,0,0,0);
        return ans;
    }
};
