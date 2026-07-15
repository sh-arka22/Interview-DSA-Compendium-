// Compile: g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <iostream>
#include <cassert>
using namespace std;

static int failures = 0;

static void check(const string& name, bool cond) {
    if (!cond) { cerr << "FAIL  " << name << "\n"; ++failures; return; }
    cout << "PASS  " << name << "\n";
}

// Small helpers to build Result values inline in expected-output lists.
static Result B(bool b) { return b; }
static Result N() { return monostate{}; }
static Result I(long long v) { return v; }
static Result TopList(vector<pair<string, long long>> v) { return v; }

static void checkResults(const string& name, const vector<Result>& got,
                          const vector<Result>& want) {
    check(name + " (size)", got.size() == want.size());
    for (size_t i = 0; i < min(got.size(), want.size()); ++i) {
        check(name + " [" + to_string(i) + "]", got[i] == want[i]);
    }
}

// ── Example 1: a completed shift contributes to total time and ranking ───────
void test_example1() {
    vector<vector<string>> ops = {
        // ["ADD", timestamp, employee_id, role, pay_rate
        {"ADD", "0", "a", "dev", "10"},
        // ["CLOCK_IN", timestamp, employee_id]
        {"CLOCK_IN", "1", "a"},
        // ["CLOCK_OUT", timestamp, employee_id]
        {"CLOCK_OUT", "6", "a"},
        // ["TOTAL_TIME", timestamp, employee_id]
        {"TOTAL_TIME", "6", "a"},
        // ["TOP_TIME", timestamp, k]
        {"TOP_TIME", "6", "1"},
    };
    vector<Result> want = {
        B(true), B(true), B(true), I(5), TopList({{"a", 5}}),
    };
    checkResults("example1", solution(ops), want);
}

// ── Example 2: duplicate employees and invalid transitions don't mutate state ─
void test_example2() {
    vector<vector<string>> ops = {
        {"ADD", "0", "a", "dev", "10"},
        {"ADD", "1", "a", "qa", "9"},          // duplicate id -> false, no mutation
        {"CLOCK_OUT", "2", "a"},               // never clocked in -> false
        {"CLOCK_IN", "2", "missing"},          // unknown id -> false
        {"TOTAL_TIME", "2", "missing"},        // unknown id -> None
    };
    vector<Result> want = {
        B(true), B(false), B(false), B(false), N(),
    };
    checkResults("example2", solution(ops), want);
}

// ── Double clock-in is rejected AND does not reset the original clock-in time ─
void test_double_clock_in_preserves_original_time() {
    vector<vector<string>> ops = {
        {"ADD", "0", "a", "dev", "10"},
        {"CLOCK_IN", "1", "a"},
        {"CLOCK_IN", "3", "a"},   // rejected; must NOT overwrite clockInTime=1 with 3
        {"CLOCK_OUT", "6", "a"},
        {"TOTAL_TIME", "6", "a"},
    };
    vector<Result> want = {
        B(true), B(true), B(false), B(true), I(5),   // 6-1=5, not 6-3=3
    };
    checkResults("double_clock_in", solution(ops), want);
}

// ── Double clock-out is rejected and does not double-count the shift ─────────
void test_double_clock_out_no_double_count() {
    vector<vector<string>> ops = {
        {"ADD", "0", "a", "dev", "10"},
        {"CLOCK_IN", "1", "a"},
        {"CLOCK_OUT", "6", "a"},
        {"CLOCK_OUT", "9", "a"},   // already clocked out -> false
        {"TOTAL_TIME", "9", "a"},
    };
    vector<Result> want = {
        B(true), B(true), B(true), B(false), I(5),
    };
    checkResults("double_clock_out", solution(ops), want);
}

// ── Multiple completed shifts accumulate; ranking key updates, not duplicates ─
void test_multiple_shifts_accumulate() {
    vector<vector<string>> ops = {
        {"ADD", "0", "a", "dev", "10"},
        {"CLOCK_IN", "1", "a"},
        {"CLOCK_OUT", "4", "a"},    // +3 -> total 3
        {"CLOCK_IN", "5", "a"},
        {"CLOCK_OUT", "11", "a"},   // +6 -> total 9
        {"TOTAL_TIME", "11", "a"},
        {"TOP_TIME", "11", "5"},
    };
    vector<Result> want = {
        B(true), B(true), B(true), B(true), B(true),
        I(9), TopList({{"a", 9}}),   // must appear exactly once, with the summed total
    };
    checkResults("multiple_shifts_accumulate", solution(ops), want);
}

// ── An employee with no completed shift is excluded from TOP_TIME entirely ───
// (not shown with a total of 0) — "b" is added and clocked in but never clocks
// out, so it must not appear even though "a" has a completed shift.
void test_incomplete_shift_excluded_from_ranking() {
    vector<vector<string>> ops = {
        {"ADD", "0", "a", "dev", "10"},
        {"ADD", "0", "b", "qa", "9"},
        {"CLOCK_IN", "1", "a"},
        {"CLOCK_OUT", "3", "a"},   // a: total 2
        {"CLOCK_IN", "1", "b"},    // b never clocks out
        {"TOP_TIME", "5", "5"},
    };
    vector<Result> want = {
        B(true), B(true), B(true), B(true), B(true),
        TopList({{"a", 2}}),   // b absent, despite k=5 asking for more
    };
    checkResults("incomplete_shift_excluded", solution(ops), want);
}

// ── TOP_TIME tie-break: equal totals order alphabetically by id ──────────────
void test_top_time_tie_break_alphabetical() {
    vector<vector<string>> ops = {
        {"ADD", "0", "charlie", "dev", "10"},
        {"ADD", "0", "alice", "dev", "10"},
        {"ADD", "0", "bob", "dev", "10"},
        {"CLOCK_IN", "0", "charlie"}, {"CLOCK_OUT", "5", "charlie"},  // 5
        {"CLOCK_IN", "0", "alice"},   {"CLOCK_OUT", "5", "alice"},    // 5
        {"CLOCK_IN", "0", "bob"},     {"CLOCK_OUT", "5", "bob"},      // 5
        {"TOP_TIME", "5", "3"},
    };
    auto results = solution(ops);
    vector<Result> want = {
        B(true), B(true), B(true),
        B(true), B(true), B(true), B(true), B(true), B(true),
        TopList({{"alice", 5}, {"bob", 5}, {"charlie", 5}}),  // alphabetical on tie
    };
    checkResults("top_time_tie_break", results, want);
}

// ── TOP_TIME with k larger than the number of eligible employees ─────────────
void test_top_time_k_exceeds_eligible() {
    vector<vector<string>> ops = {
        {"ADD", "0", "a", "dev", "10"},
        {"CLOCK_IN", "1", "a"},
        {"CLOCK_OUT", "3", "a"},
        {"TOP_TIME", "3", "10"},   // only 1 eligible employee exists
    };
    vector<Result> want = {
        B(true), B(true), B(true), TopList({{"a", 2}}),
    };
    checkResults("top_time_k_exceeds_eligible", solution(ops), want);
}

// ── TOP_TIME with k = 0 returns an empty list, not an error ──────────────────
void test_top_time_k_zero() {
    vector<vector<string>> ops = {
        {"ADD", "0", "a", "dev", "10"},
        {"CLOCK_IN", "1", "a"},
        {"CLOCK_OUT", "3", "a"},
        {"TOP_TIME", "3", "0"},
    };
    vector<Result> want = {
        B(true), B(true), B(true), TopList({}),
    };
    checkResults("top_time_k_zero", solution(ops), want);
}

// ── TOP_TIME with no eligible employees at all returns an empty list ─────────
void test_top_time_empty_state() {
    vector<vector<string>> ops = {
        {"TOP_TIME", "0", "5"},
    };
    vector<Result> want = { TopList({}) };
    checkResults("top_time_empty_state", solution(ops), want);
}

// ── Zero-duration shift (clock in and out at the same timestamp) counts as
//    completed with a total of 0, and IS eligible for ranking (it completed).
void test_zero_duration_shift_is_still_completed() {
    vector<vector<string>> ops = {
        {"ADD", "0", "a", "dev", "10"},
        {"CLOCK_IN", "5", "a"},
        {"CLOCK_OUT", "5", "a"},
        {"TOTAL_TIME", "5", "a"},
        {"TOP_TIME", "5", "5"},
    };
    vector<Result> want = {
        B(true), B(true), B(true), I(0), TopList({{"a", 0}}),
    };
    checkResults("zero_duration_shift", solution(ops), want);
}

int main() {
    cout << "-- Employee Time Tracking and Ranking --\n";
    test_example1();
    test_example2();
    test_double_clock_in_preserves_original_time();
    test_double_clock_out_no_double_count();
    test_multiple_shifts_accumulate();
    test_incomplete_shift_excluded_from_ranking();
    test_top_time_tie_break_alphabetical();
    test_top_time_k_exceeds_eligible();
    test_top_time_k_zero();
    test_top_time_empty_state();
    test_zero_duration_shift_is_still_completed();

    if (failures > 0) {
        cerr << "\n" << failures << " test(s) FAILED.\n";
        return 1;
    }
    cout << "\nAll tests passed.\n";
    return 0;
}
