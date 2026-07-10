#include <vector>
#include <deque>
#include <climits>
using namespace std;

class Solution {
public:
    // Optimal approach: 0-1 BFS.
    //
    // Model: entering a cell costs 1 if empty (0), 0 if charged (1), blocked if
    // forbidden (2). A virtual source sits above row 0 and connects to every
    // non-forbidden cell of row 0 (paying that cell's own cost — the classic bug
    // is forgetting that endpoints cost 1 too). The answer is the shortest
    // distance to any cell of the bottom row, or -1.
    //
    // With only 0/1 edge weights, Dijkstra's heap is redundant: a deque stays
    // distance-sorted for free. Relaxing a 0-edge keeps the node in the current
    // distance layer -> push_front; a 1-edge belongs to the next layer ->
    // push_back. Every pop is in nondecreasing distance order (Dijkstra's
    // invariant) but each operation is O(1), so the log factor vanishes.
    //
    // rows == 1 needs no special case: the multi-source init already prices
    // row-0 cells (0 if charged, 1 if empty, skipped if forbidden) and row 0 is
    // also the bottom row.
    //
    // O(rows * cols) time, O(rows * cols) space.
    int solution(vector<vector<int>>& g) {
        int n = g.size(), m = g[0].size();
        vector<vector<int>> dist(n, vector<int>(m, INT_MAX));
        vector<vector<char>> done(n, vector<char>(m, 0));
        deque<pair<int,int>> dq;

        // Virtual source: seed all of row 0.
        for (int c = 0; c < m; c++) if (g[0][c] != 2) {
            dist[0][c] = (g[0][c] == 0);
            if (dist[0][c] == 0) dq.push_front({0, c}); else dq.push_back({0, c});
        }

        const int dr[] = {1, -1, 0, 0}, dc[] = {0, 0, 1, -1};
        while (!dq.empty()) {
            auto [r, c] = dq.front(); dq.pop_front();
            if (done[r][c]) continue;          // stale entry (deque may hold dupes)
            done[r][c] = 1;
            if (r == n - 1) return dist[r][c]; // first bottom-row pop = optimal
            for (int k = 0; k < 4; k++) {
                int nr = r + dr[k], nc = c + dc[k];
                if (nr < 0 || nr >= n || nc < 0 || nc >= m || g[nr][nc] == 2) continue;
                int w = (g[nr][nc] == 0);      // cost of entering the neighbour
                if (dist[r][c] + w < dist[nr][nc]) {
                    dist[nr][nc] = dist[r][c] + w;
                    if (w == 0) dq.push_front({nr, nc}); else dq.push_back({nr, nc});
                }
            }
        }
        return -1; // bottom row unreachable: every route crosses a forbidden wall
    }
};
