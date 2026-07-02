// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <iostream>
using namespace std;

static void check(const string& name, long long got, long long exp) {
    if (got != exp) {
        cerr << "FAIL  " << name << "\n  got: " << got << "\n  exp: " << exp << "\n";
        exit(1);
    }
    cout << "PASS  " << name << " -> " << got << "\n";
}

Solution sol;

int main() {
    // Example 1: two short clockwise arcs.
    vector<int> c1 = {1, 2, 3, 4};
    vector<int> r1 = {0, 2, 3};
    check("ex1", sol.solution(c1, r1), 6);

    // Example 2: the two arcs are exactly equal (tie) -> either is fine.
    vector<int> r2 = {1, 3};
    check("ex2: equal arcs", sol.solution(c1, r2), 5);

    // Example 3: same-hub leg contributes 0; also visits repeat.
    vector<int> c3 = {4, 4, 4};
    vector<int> r3 = {0, 1, 1, 0};
    check("ex3: same-hub leg", sol.solution(c3, r3), 8);

    // Single stop, no legs -> distance 0.
    vector<int> r4 = {2};
    check("single stop", sol.solution(c1, r4), 0);

    // A leg that is cheaper going counterclockwise (the wraparound arc wins).
    // 0 -> 3 clockwise = 1+2+3 = 6; counterclockwise = corridor[3] = 4 -> take 4.
    vector<int> r5 = {0, 3};
    check("wraparound wins", sol.solution(c1, r5), 4);

    // Two hubs, going back and forth: each leg is min(arc, total-arc).
    vector<int> c6 = {5, 10};        // total 15; between hub 0 and 1 arcs are 5 and 10
    vector<int> r6 = {0, 1, 0};
    check("n=2 back and forth", sol.solution(c6, r6), 10);

    // 64-bit check: large ring, worst-case-ish legs must not overflow 32-bit.
    // 100000 corridors of 10000 -> loop = 1e9; a half-loop leg is 5e8. Repeat it.
    vector<int> big(100000, 10000);
    vector<int> rbig = {0, 50000, 0, 50000, 0};   // 4 legs of 5e8 each = 2e9
    check("64-bit no overflow", sol.solution(big, rbig), 2000000000LL);

    cout << "\nAll tests passed.\n";
}
