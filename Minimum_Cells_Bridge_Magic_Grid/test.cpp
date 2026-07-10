// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include "solution_dijkstra.cpp"
#include "solution_binary_search.cpp"
#include <iostream>
#include <string>
using namespace std;

Solution opt;                 // 0-1 BFS (primary)
SolutionDijkstra dij;         // approach 1
SolutionBinarySearch bs;      // approach 2

// Run all three implementations and demand they agree with each other AND the
// expected answer — cross-verification catches bugs a single oracle misses.
static void check(const string& name, vector<vector<int>> g, int exp) {
    auto g1 = g, g2 = g, g3 = g;
    int a = opt.solution(g1), b = dij.solution(g2), c = bs.solution(g3);
    if (a != exp || b != exp || c != exp) {
        cerr << "FAIL  " << name << "\n  0-1 BFS: " << a << "  Dijkstra: " << b
             << "  BinSearch: " << c << "  expected: " << exp << "\n";
        exit(1);
    }
    cout << "PASS  " << name << " -> " << a << "\n";
}

int main() {
    // Example 1: charge (1,0) and (2,0) -> path down the left column.
    check("ex1: two charges", {{1,0,0},{0,0,0},{0,0,1}}, 2);

    // Example 2: charged path already exists down the right column.
    check("ex2: already castable", {{1,1,1},{2,2,1},{1,1,1}}, 0);

    // rows == 1 trio: empty (charge it), forbidden (impossible), charged (free).
    check("1x1 empty", {{0}}, 1);
    check("1x1 forbidden", {{2}}, -1);
    check("1x1 charged", {{1}}, 0);
    check("1x3 row, pick any empty", {{0,2,0}}, 1);

    // Diagonal adjacency does NOT count -> forbidden corners block everything.
    check("diagonal is not adjacent", {{1,2},{2,1}}, -1);

    // A solid forbidden wall across the middle: impossible.
    check("forbidden wall", {{0,0},{2,2},{0,0}}, -1);

    // Wall with no charged cells anywhere: must buy the full detour (3 cells).
    check("buy full path around wall", {{0,0},{0,2},{0,0}}, 3);

    // Charged columns left+right of a forbidden spine; bridge costs 1.
    check("bridge two charged columns", {{1,2,1},{0,2,0},{1,2,1}}, 1);

    // Zig-zag: cheapest route must weave through forbidden cells.
    check("zig-zag weave", {{1,2,0},{0,2,0},{0,0,0},{2,2,1}}, 3);

    // Already-charged winding path: answer 0 even though it wanders sideways.
    check("winding charged path", {{1,2,2},{1,1,1},{2,2,1}}, 0);

    cout << "\nAll tests passed (3 implementations cross-verified).\n";
    return 0;
}
