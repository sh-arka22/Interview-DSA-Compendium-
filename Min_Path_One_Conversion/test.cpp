// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <iostream>
using namespace std;

static void check(const string& name, int got, int exp) {
    if (got != exp) {
        cerr << "FAIL  " << name << "\n  got: " << got << "\n  exp: " << exp << "\n";
        exit(1);
    }
    cout << "PASS  " << name << " -> " << got << "\n";
}

Solution sol;

int main() {
    // Example 1: a clear path already exists; no conversion needed.
    vector<vector<int>> ex1 = {{1,1,0},{0,1,0},{0,1,1}};
    check("ex1: open path", sol.solution(ex1), 4);

    // Example 2: both routes are blocked; the single conversion unlocks one step.
    vector<vector<int>> ex2 = {{1,0},{0,1}};
    check("ex2: convert to pass", sol.solution(ex2), 2);

    // All open: answer is just the Manhattan distance, conversion unused.
    vector<vector<int>> open3 = {{1,1,1},{1,1,1},{1,1,1}};
    check("all open 3x3", sol.solution(open3), 4);

    // A full vertical wall (column 1 blocked) — impossible without a conversion,
    // solvable by punching exactly one hole. Optimal = Manhattan distance 4.
    vector<vector<int>> wall = {{1,0,1},{1,0,1},{1,0,1}};
    check("wall needs one hole", sol.solution(wall), 4);

    // Two barriers to cross: a single conversion is not enough -> -1.
    vector<vector<int>> imposs = {{1,0,0},{0,0,0},{0,0,1}};
    check("two walls impossible", sol.solution(imposs), -1);

    // Blocked start on a 1x1 grid: convert the start, already at the goal.
    vector<vector<int>> start0 = {{0}};
    check("blocked 1x1 start", sol.solution(start0), 0);

    // The one conversion may be spent on the destination cell itself.
    vector<vector<int>> endconv = {{1,1},{1,0}};
    check("convert the end cell", sol.solution(endconv), 2);

    cout << "\nAll tests passed.\n";
}
