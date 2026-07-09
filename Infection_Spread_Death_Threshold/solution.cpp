#include <vector>
using namespace std;

class Solution {
public:
    vector<vector<int>> solution(vector<vector<int>> grid, int infectThreshold, int deathThreshold) {
        int m = grid.size(), n = m ? grid[0].size() : 0;
        static const int dx[8] = {-1,-1,-1,0,0,1,1,1};
        static const int dy[8] = {-1,0,1,-1,1,-1,0,1};
        auto inB = [&](int x,int y){ return x>=0 && x<m && y>=0 && y<n; };

        // Cells infected in the INITIAL grid are treated as NOT marked for death (per spec),
        // so we only seed the frontier; markedForDeath stays false for them. Only a
        // 0 -> infected transition can set the death mark.
        vector<vector<bool>> markedForDeath(m, vector<bool>(n,false));
        vector<pair<int,int>> frontier;
        for (int i=0;i<m;i++) for(int j=0;j<n;j++) if (grid[i][j]==1) frontier.push_back({i,j});

        vector<vector<int>> cnt(m, vector<int>(n,0));       // reused across rounds, cleared as we go
        while (!frontier.empty()) {
            vector<pair<int,int>> touched;
            for (auto& p : frontier) {                       // each frontier cell votes exactly once, ever
                int x=p.first, y=p.second;
                for (int d=0; d<8; d++) {
                    int nx=x+dx[d], ny=y+dy[d];
                    if (!inB(nx,ny) || grid[nx][ny] != 0) continue;  // only 0-cells are eligible
                    if (cnt[nx][ny] == 0) touched.push_back({nx,ny});
                    cnt[nx][ny]++;
                }
            }
            vector<pair<int,int>> nextFrontier;
            for (auto& p : touched) {
                int x=p.first, y=p.second;
                if (cnt[x][y] >= infectThreshold) {
                    nextFrontier.push_back({x,y});
                    markedForDeath[x][y] = (cnt[x][y] >= deathThreshold);   // same c, both checks
                }
                cnt[x][y] = 0;                               // discard this round's tally, ready for reuse
            }
            for (auto& p : frontier)     grid[p.first][p.second] = markedForDeath[p.first][p.second] ? 3 : 2;
            for (auto& p : nextFrontier) grid[p.first][p.second] = 1;

            frontier = std::move(nextFrontier);
        }
        return grid;
    }
};
