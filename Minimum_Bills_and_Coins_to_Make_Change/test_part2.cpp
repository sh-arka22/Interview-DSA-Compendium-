// Compile: g++ -std=c++17 -O2 -o test_part2 test_part2.cpp && ./test_part2
#include "solution_part2.cpp"
#include <iostream>
#include <string>
using namespace std;

static void check(const string& name, bool cond) {
    if (!cond) { cerr << "FAIL  " << name << "\n"; exit(1); }
    cout << "PASS  " << name << "\n";
}

// ── Ample stock reproduces the unlimited-greedy answer ────────────────────────
void test_ample_stock() {
    vector<int> inv = {10, 10, 10, 10, 10, 10, 10, 10};
    int got = Solution().minPiecesLimited(6.35, inv);
    check("ample_stock: 6.35 -> " + to_string(got), got == 4);
}

// ── Zero change needs zero pieces, even with an empty drawer ──────────────────
void test_zero_amount_empty_drawer() {
    vector<int> inv = {0, 0, 0, 0, 0, 0, 0, 0};
    int got = Solution().minPiecesLimited(0.0, inv);
    check("zero_amount: 0.00 empty drawer -> " + to_string(got), got == 0);
}

// ── The headline greedy-fails / DP-succeeds counterexample ────────────────────
// amount = $0.30, inventory has one $0.25 and three $0.10 (nothing else).
// Greedy takes the quarter, strands $0.05, and would report -1.
// DP finds three dimes = $0.30 in 3 pieces.
void test_greedy_fails_dp_succeeds() {
    vector<int> inv = {0, 0, 0, 0, 1, 3, 0, 0};
    int got = Solution().minPiecesLimited(0.30, inv);
    check("greedy_trap: 0.30 with {1x25c,3x10c} -> " + to_string(got), got == 3);
}

// ── Genuinely infeasible — must return -1, not crash or under-count ───────────
void test_infeasible() {
    vector<int> inv = {0, 0, 0, 0, 0, 0, 0, 0};
    int got = Solution().minPiecesLimited(0.05, inv);
    check("infeasible: 0.05 empty drawer -> " + to_string(got), got == -1);
}

// ── Exact stock, no slack — every coin must be used correctly ─────────────────
void test_exact_stock_no_slack() {
    vector<int> inv = {0, 0, 0, 0, 0, 0, 0, 5};
    int got = Solution().minPiecesLimited(0.05, inv);
    check("exact_stock: 0.05 with exactly 5 pennies -> " + to_string(got), got == 5);
}

// ── One short of enough must fail cleanly ──────────────────────────────────────
void test_one_short() {
    vector<int> inv = {0, 0, 0, 0, 0, 0, 0, 4};
    int got = Solution().minPiecesLimited(0.05, inv);
    check("one_short: 0.05 with only 4 pennies -> " + to_string(got), got == -1);
}

// ── Large stock exercises binary splitting across many parcels ────────────────
void test_large_stock_binary_split() {
    vector<int> inv = {0, 0, 0, 0, 0, 1000, 0, 0};   // 1000 dimes, nothing else
    int got = Solution().minPiecesLimited(5.00, inv); // 500 cents / 10 = 50 dimes
    check("binary_split: 5.00 from up to 1000 dimes -> " + to_string(got), got == 50);
}

// ── Mixed denominations where the cheapest path skips the obvious largest ─────
// amount = $0.40, inventory = {1x $0.25, 1x $0.10, 1x $0.05} -> must use all three
void test_forced_full_use() {
    vector<int> inv = {0, 0, 0, 0, 1, 1, 1, 0};
    int got = Solution().minPiecesLimited(0.40, inv);
    check("forced_full_use: 0.40 -> " + to_string(got), got == 3);
}

// ── Amount larger than any single-denomination stock can cover alone ──────────
void test_requires_multiple_denominations() {
    // $27 with only 1x $20 and unlimited-ish $1s (10) and no $10/$5
    vector<int> inv = {1, 0, 0, 10, 0, 0, 0, 0};
    int got = Solution().minPiecesLimited(27.00, inv);
    check("multi_denom: 27.00 -> " + to_string(got), got == 8); // 1x$20 + 7x$1
}

int main() {
    cout << "-- Part 2: bounded knapsack DP over limited inventory --\n";
    test_ample_stock();
    test_zero_amount_empty_drawer();
    test_greedy_fails_dp_succeeds();
    test_infeasible();
    test_exact_stock_no_slack();
    test_one_short();
    test_large_stock_binary_split();
    test_forced_full_use();
    test_requires_multiple_denominations();
    cout << "\nAll tests passed.\n";
    return 0;
}
