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
        for (int s = 0; s < (1 << n); s++) {
            int combined = 0, len = 0;
            for (int i = 0; i < n; i++) {
                if (s >> i & 1) {
                    combined |= masks[i];
                    len     += __builtin_popcount(masks[i]);
                }
            }
            if (__builtin_popcount(combined) == len)
                ans = max(ans, len);
        }
        return ans;
    }
};
