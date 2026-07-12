#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
using namespace std;

class Solution {
public:
    // Diagonals run bottom-left -> top-right (r decreases, c increases).
    // Indexing convention (reverse-engineered from Example 1, [0,2,1]):
    // starting cells are enumerated from the bottom-left corner ALONG THE
    // BOTTOM ROW left->right, then UP THE LEFT COLUMN:
    //   i < cols      : start (rows-1, i)
    //   i >= cols     : start (rows+cols-2-i, 0)
    // Each diagonal's string is read walking up-right from its start.
    // Sort indices by string; ties keep smaller index (stable sort).
    //
    // O(R*C) to build all strings (every cell appears in exactly one diagonal)
    // + O(D log D) comparisons, D = R + C - 1.
    vector<int> solution(vector<vector<char>>& g) {
        int n = g.size(), m = g[0].size(), D = n + m - 1;
        vector<string> s(D);
        for (int i = 0; i < D; i++) {
            int r = (i < m) ? n - 1 : n + m - 2 - i;
            int c = (i < m) ? i : 0;
            for (; r >= 0 && c < m; r--, c++) s[i] += g[r][c];
        }
        vector<int> idx(D);
        iota(idx.begin(), idx.end(), 0);
        stable_sort(idx.begin(), idx.end(),
                    [&](int a, int b) { return s[a] < s[b]; });
        return idx;
    }
};
