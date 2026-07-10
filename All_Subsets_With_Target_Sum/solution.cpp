#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
    vector<vector<int>> solution(vector<int> nums, int target) {
        // Sort ascending: (1) DFS then emits subsets in lexicographic order
        // for free, no post-sort needed; (2) enables the "break as soon as a
        // value exceeds the remaining sum" prune (all later values are >=).
        sort(nums.begin(), nums.end());
        int n = nums.size();

        // Feasibility DP over suffixes (the key optimization):
        // reach[i][t] == true  iff  some subset of nums[i..n-1] sums to exactly t.
        // Built right-to-left in O(n * target). During the DFS we only step into
        // a branch if the remaining amount is still achievable from the suffix,
        // so we NEVER enter a dead subtree — every visited node lies on a path
        // to at least one emitted answer (output-sensitive enumeration).
        vector<vector<char>> reach(n + 1, vector<char>(target + 1, 0));
        reach[n][0] = 1;
        for (int t = 0; t <= target; t++){
            for (int i = n - 1; i >= 0; i--){
                reach[i][t] = reach[i + 1][t] ||
                              (t >= nums[i] && reach[i + 1][t - nums[i]]);
            }
        }

        vector<vector<int>> ans;
        vector<int> path;

        // Classic index-based combination DFS: each index used at most once,
        // equal values at different indexes are distinct choices (no dedup).
        auto dfs = [&](auto&& self, int start, int remaining) -> void {
            if (remaining == 0) ans.push_back(path); // record; zeros may still extend
            for (int i = start; i < n; i++) {
                if (nums[i] > remaining) break;                    // sorted: all later fail too
                if (!reach[i + 1][remaining - nums[i]]) continue;  // provably dead: skip
                path.push_back(nums[i]);
                self(self, i + 1, remaining - nums[i]);
                path.pop_back();
            }
        };

        if (reach[0][target]) dfs(dfs, 0, target);

        // DFS over the sorted array emits answers already in lexicographic
        // order EXCEPT one edge case: duplicate zeros (e.g. it yields
        // [0],[0,0],[0] instead of [0],[0],[0,0]). The list is near-sorted,
        // so this final sort is effectively linear and guarantees the contract.
        sort(ans.begin(), ans.end());
        return ans;
    }
};
