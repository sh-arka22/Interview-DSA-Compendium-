#include <string>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;

class Solution {
public:
    // Key insight: abbr = prefix + middleCount + lastChar parses uniquely (a
    // digit can never be a word letter), so two abbreviations are equal iff the
    // words share (length, last char) AND their first k letters. Therefore:
    //   1. Only words in the same (length, lastChar) group can ever conflict.
    //   2. The iterative "bump k on conflict" process converges directly to
    //        k(word) = 1 + max LCP with any other word in its group.
    //   3. Within a sorted group, the max LCP is achieved with an adjacent
    //      neighbour -> only n-1 adjacent LCPs per group are needed.
    // Keep the original word when the abbreviation is not shorter.
    //
    // O(N L log N) time, O(N L) space.  N, L <= 400.
    vector<string> solution(vector<string>& words) {
        int n = words.size();
        map<pair<int,char>, vector<int>> groups;          // (len, last) -> indices
        for (int i = 0; i < n; i++)
            groups[{(int)words[i].size(), words[i].back()}].push_back(i);

        auto abbr = [](const string& w, int k) {
            string a = w.substr(0, k) + to_string((int)w.size() - k - 1) + w.back();
            return a.size() < w.size() ? a : w;           // not shorter -> keep word
        };
        auto lcp = [](const string& a, const string& b) {
            int i = 0, m = min(a.size(), b.size());
            while (i < m && a[i] == b[i]) i++;
            return i;
        };

        vector<string> res(n);
        for (auto& [key, idx] : groups) {
            sort(idx.begin(), idx.end(),
                 [&](int a, int b) { return words[a] < words[b]; });
            int m = idx.size();
            vector<int> L(m + 1, 0);                      // L[i] = LCP(sorted[i-1], sorted[i])
            for (int i = 1; i < m; i++) L[i] = lcp(words[idx[i - 1]], words[idx[i]]);
            for (int i = 0; i < m; i++)                   // k = 1 + max(neighbour LCPs)
                res[idx[i]] = abbr(words[idx[i]], 1 + max(L[i], L[i + 1]));
        }
        return res;
    }
};
