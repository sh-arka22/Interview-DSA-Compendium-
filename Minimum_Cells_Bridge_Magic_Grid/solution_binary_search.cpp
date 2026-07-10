#include <vector>
#include <stack>
#include <utility>
using namespace std;

class SolutionBinarySearch {
    // Approach 2: binary search on the answer + layered DFS feasibility check.
    //
    // The predicate check(k) = "the bottom row is reachable charging at most k
    // empty cells" is monotone in k, so the minimum feasible k can be binary
    // searched.
    //
    // check(k) works like peeling an onion in reverse:
    //   1. Flood for FREE through charged cells (DFS) starting from row 0.
    //   2. The flood's border of empty cells is the "boundary". Buying the whole
    //      boundary costs exactly 1 rune of path-depth: any path that leaves the
    //      flooded region must step through one boundary cell first.
    //   3. Buy the boundary, flood again, repeat at most k times.
    // Each cell is visited at most once per check -> O(RC) per check.
    //
    // O(RC log(RC)) total, O(RC) space. Asymptotically ties Dijkstra but with a
    // larger constant (each probe re-scans the grid). Included because
    // "binary search the answer" is a reusable interview pattern whenever the
    // feasibility predicate is monotone and cheaper than the direct optimum.
    int n, m;
    vector<vector<int>>* G;

    bool check(int k) {
        auto& g = *G;
        vector<vector<char>> vis(n, vector<char>(m, 0));
        bool reached = false;
        vector<pair<int,int>> boundary;   // empty cells adjacent to the flooded region
        stack<pair<int,int>> st;
        const int dr[] = {1, -1, 0, 0}, dc[] = {0, 0, 1, -1};

        auto flood = [&](vector<pair<int,int>>& nextB) {  // free DFS through charged cells
            while (!st.empty()) {
                auto [r, c] = st.top(); st.pop();
                if (r == n - 1) reached = true;
                for (int t = 0; t < 4; t++) {
                    int nr = r + dr[t], nc = c + dc[t];
                    if (nr < 0 || nr >= n || nc < 0 || nc >= m || vis[nr][nc] || g[nr][nc] == 2) continue;
                    if (g[nr][nc] == 1) { vis[nr][nc] = 1; st.push({nr, nc}); }
                    else nextB.push_back({nr, nc});       // empty: candidate to buy later
                }
            }
        };

        // Seed from row 0: charged cells flood free; empty cells are boundary.
        for (int c = 0; c < m; c++) {
            if (g[0][c] == 1 && !vis[0][c]) { vis[0][c] = 1; st.push({0, c}); }
            else if (g[0][c] == 0) boundary.push_back({0, c});
        }
        flood(boundary);
        if (reached) return true;

        for (int b = 1; b <= k && !boundary.empty(); b++) {  // spend 1 rune per layer
            vector<pair<int,int>> nextB;
            for (auto [r, c] : boundary)
                if (!vis[r][c]) { vis[r][c] = 1; st.push({r, c}); }  // buy this empty cell
            flood(nextB);
            if (reached) return true;
            boundary = move(nextB);
        }
        return reached;
    }

public:
    int solution(vector<vector<int>>& g) {
        n = g.size(); m = g[0].size(); G = &g;
        int lo = 0, hi = n * m;
        if (!check(hi)) return -1;        // unreachable even with unlimited runes
        while (lo < hi) {                 // smallest k with check(k) == true
            int mid = lo + (hi - lo) / 2;
            if (check(mid)) hi = mid; else lo = mid + 1;
        }
        return lo;
    }
};
