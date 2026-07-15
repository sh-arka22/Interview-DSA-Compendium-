// Compile: g++ -std=c++17 -O2 -o test_part1 test_part1.cpp && ./test_part1
#include "solution_part1.cpp"
#include <iostream>
#include <string>
using namespace std;

static void check(const string& name, bool cond) {
    if (!cond) { cerr << "FAIL  " << name << "\n"; exit(1); }
    cout << "PASS  " << name << "\n";
}

static string show(const vector<int>& v) {
    string s = "[";
    for (size_t i = 0; i < v.size(); ++i) s += (i ? "," : "") + to_string(v[i]);
    return s + "]";
}

// ── Example 1: $6.35 -> $5 + $1 + $0.25 + $0.10 = 4 pieces ───────────────────
void test_example1() {
    vector<int> got = Solution().minPieces(6.35);
    vector<int> want = {4, 0, 0, 1, 1, 1, 1, 0, 0};
    check("ex1: 6.35 -> " + show(got), got == want);
}

// ── Example 2: zero change needs zero pieces ──────────────────────────────────
void test_example2() {
    vector<int> got = Solution().minPieces(0.0);
    vector<int> want = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    check("ex2: 0.00 -> " + show(got), got == want);
}

// ── Floating-point trap: 0.30 must land on exactly one quarter + one nickel ──
void test_floating_point_trap() {
    vector<int> got = Solution().minPieces(0.30);
    vector<int> want = {2, 0, 0, 0, 0, 1, 0, 1, 0};
    check("fp_trap: 0.30 -> " + show(got), got == want);
}

// ── Single largest denomination, nothing smaller needed ───────────────────────
void test_exact_bill() {
    vector<int> got = Solution().minPieces(20.00);
    vector<int> want = {1, 1, 0, 0, 0, 0, 0, 0, 0};
    check("exact_bill: 20.00 -> " + show(got), got == want);
}

// ── Amount requiring every denomination at least once ─────────────────────────
void test_all_denominations() {
    // 20+10+5+1+0.25+0.10+0.05+0.01 = 36.41 -> exactly one of each
    vector<int> got = Solution().minPieces(36.41);
    vector<int> want = {8, 1, 1, 1, 1, 1, 1, 1, 1};
    check("all_denoms: 36.41 -> " + show(got), got == want);
}

// ── Odd cent amount that forces every coin down to the penny ─────────────────
void test_penny_forced() {
    // 0.03 -> only reachable via 3 pennies (no nickel/dime/quarter combo works)
    vector<int> got = Solution().minPieces(0.03);
    vector<int> want = {3, 0, 0, 0, 0, 0, 0, 0, 3};
    check("penny_forced: 0.03 -> " + show(got), got == want);
}

int main() {
    cout << "-- Part 1: greedy over unlimited canonical denominations --\n";
    test_example1();
    test_example2();
    test_floating_point_trap();
    test_exact_bill();
    test_all_denominations();
    test_penny_forced();
    cout << "\nAll tests passed.\n";
    return 0;
}
