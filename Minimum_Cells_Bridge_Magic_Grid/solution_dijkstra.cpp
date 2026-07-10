#include <vector>
#include <queue>
#include <tuple>
#include <climits>
using namespace std;

class SolutionDijkstra {
public:
    // Approach 1: textbook Dijkstra with a min-heap.
    //
    // Same weighted-grid model as solution.cpp (enter empty = 1, charged = 0,
    // forbidden = wall; multi-source from row 0). Dijkstra applies because all
    // weights are non-negative. Early-return on the first bottom-row pop —
    // Dijkstra pops in nondecreasing distance order, so it is already optimal.
    //
    // O(RC log(RC)) time, O(RC) space. Correct, but the heap pays a log factor
    // that 0-1 BFS (solution.cpp) avoids. ~3x slower in practice at 1000x1000.
    int solution(vector<vector<int>>& g) {
        int n = g.size(), m = g[0].size();
        vector<vector<int>> dist(n, vector<int>(m, INT_MAX));
        priority_queue<tuple<int,int,int>, vector<tuple<int,int,int>>, greater<>> pq;

        for (int c = 0; c < m; c++) if (g[0][c] != 2) {
            dist[0][c] = (g[0][c] == 0);
            pq.push({dist[0][c], 0, c});
        }

        const int dr[] = {1, -1, 0, 0}, dc[] = {0, 0, 1, -1};
        while (!pq.empty()) {
            auto [d, r, c] = pq.top(); pq.pop();
            if (d > dist[r][c]) continue;   // stale (lazy deletion)
            if (r == n - 1) return d;       // first bottom-row pop = optimal
            for (int k = 0; k < 4; k++) {
                int nr = r + dr[k], nc = c + dc[k];
                if (nr < 0 || nr >= n || nc < 0 || nc >= m || g[nr][nc] == 2) continue;
                int nd = d + (g[nr][nc] == 0);
                if (nd < dist[nr][nc]) { 
                    dist[nr][nc] = nd; 
                    pq.push({nd, nr, nc}); 
                }
            }
        }
        return -1;
    }
};
