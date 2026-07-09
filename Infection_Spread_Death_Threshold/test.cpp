// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <iostream>
using namespace std;

using Grid = vector<vector<int>>;

static string show(const Grid& g) {
    string s = "[";
    for (size_t i = 0; i < g.size(); i++) {
        s += "[";
        for (size_t j = 0; j < g[i].size(); j++) {
            s += to_string(g[i][j]);
            if (j + 1 < g[i].size()) s += ",";
        }
        s += "]";
    }
    return s + "]";
}

static void check(const string& name, const Grid& got, const Grid& exp) {
    if (got != exp) {
        cerr << "FAIL  " << name << "\n  got: " << show(got) << "\n  exp: " << show(exp) << "\n";
        exit(1);
    }
    cout << "PASS  " << name << " -> " << show(got) << "\n";
}

Solution sol;

int main() {
    // Example 1: seed recovers to 2; everything it infects has c>=deathThreshold and dies.
    check("ex1",
          sol.solution({{1,0,0},{0,0,0},{0,0,0}}, 1, 1),
          {{2,3,3},{3,3,3},{3,3,3}});

    // Example 2: infectThreshold=1 but deathThreshold=2; each ring cell has only 1
    // infected neighbor, so it infects yet is NOT marked -> all recover to immune.
    check("ex2",
          sol.solution({{0,0,0},{0,1,0},{0,0,0}}, 1, 2),
          {{2,2,2},{2,2,2},{2,2,2}});

    // A cell infected in the INITIAL grid is treated as unmarked -> immune, never dead,
    // even when the thresholds would otherwise mark a freshly-infected cell.
    check("initial-infected-unmarked",
          sol.solution({{1}}, 1, 1),
          {{2}});

    // No infected cells anywhere -> already stable, returned unchanged in one pass.
    check("no-infection-stable",
          sol.solution({{0,0},{0,0}}, 1, 1),
          {{0,0},{0,0}});

    // Infection spreads but deathThreshold is unreachable (=8) -> everyone recovers to 2.
    // 1x3: [1,0,1] -> center gets c=2>=infect, c<death -> infects unmarked -> immune.
    check("spread-then-immune",
          sol.solution({{1,0,1}}, 2, 8),
          {{2,2,2}});

    // Center is an initial seed (unmarked -> immune); both neighbors get infected with
    // c=1>=deathThreshold so they are marked and die. Classic "survivor in the middle".
    check("neighbors-die-center-lives",
          sol.solution({{0,1,0}}, 1, 1),
          {{3,2,3}});

    // Below infectThreshold: single infected neighbor (c=1) can't infect when
    // infectThreshold=2. Infection dies out; susceptibles stay 0, seed recovers to 2.
    check("below-threshold-dies-out",
          sol.solution({{0,1,0}}, 2, 1),
          {{0,2,0}});

    // Two adjacent INITIAL infecteds with deathThreshold=1: they each have an infected
    // neighbor, but initial-grid infecteds are NOT marked -> both recover to immune, not dead.
    check("initial-neighbors-never-marked",
          sol.solution({{1,1}}, 1, 1),
          {{2,2}});

    cout << "\nAll tests passed.\n";
}
