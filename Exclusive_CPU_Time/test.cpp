// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <iostream>
using namespace std;

static string show(const vector<int>& v) {
    string s = "[";
    for (size_t i = 0; i < v.size(); i++) { if (i) s += ", "; s += to_string(v[i]); }
    return s + "]";
}

static void check(const string& name, vector<int> got, vector<int> exp) {
    if (got != exp) {
        cerr << "FAIL  " << name << "\n  got: " << show(got) << "\n  exp: " << show(exp) << "\n";
        exit(1);
    }
    cout << "PASS  " << name << "\n";
}

Solution sol;

int main() {
    // Example 1: one nested call.
    check("ex1: nested pair",
        sol.solution(2, {"0:start:0", "1:start:2", "1:end:5", "0:end:6"}), {3, 4});

    // Example 2: starts and ends on the same (inclusive) timestamp.
    check("ex2: single unit",
        sol.solution(1, {"0:start:0", "0:end:0"}), {1});

    // Recursion: the same id appears twice on the stack.
    // times 0,1 outer | 2..5 inner | 6 outer  -> 7 units total for fn 0.
    check("recursion same id",
        sol.solution(1, {"0:start:0", "0:start:2", "0:end:5", "0:end:6"}), {7});

    // Two back-to-back (non-nested) calls: nothing on the stack in between.
    check("sequential calls",
        sol.solution(2, {"0:start:0", "0:end:2", "1:start:3", "1:end:5"}), {3, 3});

    // Three levels deep: each frame owns exactly its own two boundary units.
    check("triple nesting",
        sol.solution(3, {"0:start:0", "1:start:1", "2:start:2",
                         "2:end:3", "1:end:4", "0:end:5"}), {2, 2, 2});

    // Single function occupying a whole span [2, 9] inclusive.
    check("full span",
        sol.solution(1, {"0:start:2", "0:end:9"}), {8});

    cout << "\nAll tests passed.\n";
}
