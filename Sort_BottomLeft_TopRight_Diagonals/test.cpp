// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <iostream>
using namespace std;

Solution sol;

static string join(const vector<int>& v) {
    string s = "[";
    for (size_t i = 0; i < v.size(); i++) s += (i ? "," : "") + to_string(v[i]);
    return s + "]";
}

static void check(const string& name, vector<vector<char>> g, vector<int> exp) {
    auto got = sol.solution(g);
    if (got != exp) {
        cerr << "FAIL  " << name << "\n  got: " << join(got) << "\n  exp: " << join(exp) << "\n";
        exit(1);
    }
    cout << "PASS  " << name << " -> " << join(got) << "\n";
}

int main() {
    // Example 1: diag0="ba" (from (1,0)), diag1="d", diag2="c"
    //            -> sorted: "ba"(0) < "c"(2) < "d"(1).
    check("ex1", {{'c','a'},{'b','d'}}, {0,2,1});

    // Example 2: single cell, single diagonal.
    check("ex2", {{'a'}}, {0});

    // 3x3 hand-computed:
    //   c a t        diag0 (2,0)up-right = "fet"   diag3 (1,0) = "ba"
    //   b e d        diag1 (2,1)         = "id"    diag4 (0,0) = "c"
    //   f i g        diag2 (2,2)         = "g"
    // sorted: ba(3) < c(4) < fet(0) < g(2) < id(1)
    check("3x3", {{'c','a','t'},{'b','e','d'},{'f','i','g'}}, {3,4,0,2,1});

    // Ties broken by diagonal index: "a"(1) == "a"(2), then "aa"(0).
    check("all equal ties", {{'a','a'},{'a','a'}}, {1,2,0});

    // Single row: every diagonal is one cell, bottom row L->R indexing = column.
    check("1xN row", {{'d','b','c','a'}}, {3,1,2,0});

    // Single column: diag0 = bottom cell 'c', diag1 = 'b', diag2 = 'd' (upward)
    // -> sorted: b(1) < c(0) < d(2).
    check("Nx1 column", {{'d'},{'b'},{'c'}}, {1,0,2});

    // Prefix rule: "a" < "ab" (shorter string that is a prefix sorts first).
    //   a b     diag0 (1,0)="ca"  diag1 (1,1)="b"?? recompute:
    //   c b     grid {{'a','b'},{'c','b'}}: diag0="cb", diag1="b", diag2="a"
    // sorted: a(2) < b(1) < cb(0)
    check("prefix ordering", {{'a','b'},{'c','b'}}, {2,1,0});

    cout << "\nAll tests passed.\n";
    return 0;
}
