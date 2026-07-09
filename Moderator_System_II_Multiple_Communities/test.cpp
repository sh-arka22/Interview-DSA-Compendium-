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
        "c1,alice,added,system,1", "c1,bob,added,alice,2", "c2,bob,added,system,3",
        "c1,carol,added,alice,4", "c1,bob,removed,alice,5", "c1,bob,added,carol,6"
    };
    vector<vector<string>> queries = {
        {"getModRanking", "c1"},
        {"getModRanking", "c2"},
        {"canRemoveMod", "c1", "bob", "alice"},
        {"canRemoveMod", "c1", "carol", "bob"},
        {"canRemoveMod", "c2", "bob", "alice"}
    };
    auto got = sol.solution(logs, queries);
    check("example1", got, {
        "[\"alice\", \"carol\", \"bob\"]", "[\"bob\"]", "true", "false", "false"
    });
}

void test_example2_unknown_community() {
    vector<string> logs = {};
    vector<vector<string>> queries = { {"getModRanking", "nope"} };
    auto got = sol.solution(logs, queries);
    check("example2_unknown_community", got, { "[]" });
}

void test_independent_ranks_same_user_diff_communities() {
    // Same user can hold different (independent) ranks in different communities.
    vector<string> logs = {
        "c1,x,added,s,1", "c2,x,added,s,2", "c1,y,added,s,3", "c2,y,added,s,4"
    };
    vector<vector<string>> queries = {
        {"canRemoveMod", "c1", "y", "x"},   // x outranks y in c1 (added earlier)
        {"canRemoveMod", "c2", "y", "x"}    // same relative order holds in c2 too
    };
    auto got = sol.solution(logs, queries);
    check("independent_ranks", got, { "true", "true" });
}

int main() {
    test_example1();
    test_example2_unknown_community();
    test_independent_ranks_same_user_diff_communities();
    cout << "\nAll tests passed.\n";
}
