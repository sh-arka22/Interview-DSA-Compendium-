#include <vector>
#include <queue>
#include <array>
using namespace std;

class Solution {
public:
    // Innovative framing: a *product-graph* BFS.
    //
    // Instead of hacking a "have I converted yet?" flag into the search, we walk
    // on   grid × A   where A is a tiny 2-state automaton for the conversion budget:
    //   layer 0 = conversion still available,
    //   layer 1 = conversion already spent.
    // Stepping onto an OPEN cell keeps the layer. Stepping onto a BLOCKED cell is
    // only legal from layer 0, and it is exactly the edge layer 0 -> layer 1.
    // Two stacked copies of the grid, BFS once. (Bump A to k+1 states for "at most
    // k conversions" — the code below is unchanged in spirit.)
    int solution(vector<vector<int>>& grid) {
        int m = grid.size(), n = grid[0].size();
        auto id = [&](int r, int c, int k) { return (r * n + c) * 2 + k; };
        vector<int> dist(m * n * 2, -1);

        // Entering the start already costs the conversion if (0,0) itself is blocked.
        int k0 = grid[0][0] == 0 ? 1 : 0;
        queue<array<int, 3>> q;
        dist[id(0, 0, k0)] = 0;
        q.push({0, 0, k0});

        const int dr[] = {-1, 1, 0, 0}, dc[] = {0, 0, -1, 1};
        while (!q.empty()) {
            auto [r, c, k] = q.front(); q.pop();
            int d = dist[id(r, c, k)];
            if (r == m - 1 && c == n - 1) return d;   // BFS: first arrival is optimal

            for (int i = 0; i < 4; i++) {
                int nr = r + dr[i], nc = c + dc[i];
                if (nr < 0 || nr >= m || nc < 0 || nc >= n) continue;

                int nk = k;
                if (grid[nr][nc] == 0) {              // blocked: needs the conversion
                    if (k == 1) continue;             // budget already spent
                    nk = 1;                           // spend it -> jump to layer 1
                }
                if (dist[id(nr, nc, nk)] != -1) continue;
                dist[id(nr, nc, nk)] = d + 1;
                q.push({nr, nc, nk});
            }
        }
        return -1;
    }
};
