// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <iostream>
using namespace std;

static void check(const string& name, const vector<string>& got, const vector<string>& exp) {
    if (got != exp) {
        cerr << "FAIL  " << name << "\n  got: [";
        for (auto& s : got) cerr << s << "|";
        cerr << "]\n  exp: [";
        for (auto& s : exp) cerr << s << "|";
        cerr << "]\n";
        exit(1);
    }
    cout << "PASS  " << name << "\n";
}

Solution sol;

void test_example1() {
    vector<string> logs = {
        "c1,user1,added,s,1", "c1,user2,added,s,2", "c1,user3,added,s,3"
    };
    vector<vector<string>> queries = {
        {"getModRanking", "c1"},
        {"demote", "c1", "user1"},
        {"getModRanking", "c1"},
        {"demote", "c1", "user1"},
        {"getModRanking", "c1"}
    };
    auto got = sol.solution(logs, queries);
    check("example1", got, {
        "[\"user1\", \"user2\", \"user3\"]",
        "null",
        "[\"user2\", \"user1\", \"user3\"]",
        "null",
        "[\"user2\", \"user3\", \"user1\"]"
    });
}

void test_example2_noop_cases() {
    vector<string> logs = {
        "c1,user1,added,s,1", "c1,user2,added,s,2", "c1,user3,added,s,3"
    };
    vector<vector<string>> queries = {
        {"demote", "c1", "user1"},
        {"demote", "c1", "user3"},           // already lowest -> no-op
        {"getModRanking", "c1"},
        {"demote", "c1", "ghost"},           // not a mod -> no-op
        {"getModRanking", "c1"}
    };
    auto got = sol.solution(logs, queries);
    check("example2_noop_cases", got, {
        "null", "null",
        "[\"user2\", \"user1\", \"user3\"]",
        "null",
        "[\"user2\", \"user1\", \"user3\"]"
    });
}

// canRemoveMod must use the maintained (demoted) order, not the original timestamp order.
void test_canRemoveMod_uses_maintained_order() {
    vector<string> logs = {
        "c1,user1,added,s,1", "c1,user2,added,s,2", "c1,user3,added,s,3"
    };
    vector<vector<string>> queries = {
        {"demote", "c1", "user1"},           // -> user2, user1, user3
        {"demote", "c1", "user1"},           // -> user2, user3, user1
        {"canRemoveMod", "c1", "user1", "user3"},   // user3 now outranks user1 -> true
        {"canRemoveMod", "c1", "user3", "user1"}    // reverse -> false
    };
    auto got = sol.solution(logs, queries);
    check("canRemoveMod_maintained_order", got, { "null", "null", "true", "false" });
}

int main() {
    test_example1();
    test_example2_noop_cases();
    test_canRemoveMod_uses_maintained_order();
    cout << "\nAll tests passed.\n";
}
