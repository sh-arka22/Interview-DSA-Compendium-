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
        "alice,added,system,1", "bob,added,alice,2", "carol,added,alice,3",
        "bob,removed,alice,4", "bob,added,carol,5"
    };
    vector<vector<string>> queries = {
        {"getModRanking"},
        {"canRemoveMod", "bob", "alice"},
        {"canRemoveMod", "carol", "bob"},
        {"canRemoveMod", "alice", "alice"},
        {"canRemoveMod", "dave", "alice"}
    };
    auto got = sol.solution(logs, queries);
    check("example1", got, {
        "[\"alice\", \"carol\", \"bob\"]", "true", "false", "false", "false"
    });
}

void test_example2_empty() {
    vector<string> logs = {};
    vector<vector<string>> queries = { {"getModRanking"} };
    auto got = sol.solution(logs, queries);
    check("example2_empty", got, { "[]" });
}

// Extra: re-added user gets a fresh, lowest rank (already covered by example1's bob,
// but check it in isolation with 2 re-adds).
void test_multi_readd() {
    vector<string> logs = {
        "a,added,s,1", "b,added,s,2", "a,removed,s,3", "a,added,b,4"
    };
    vector<vector<string>> queries = { {"getModRanking"}, {"canRemoveMod", "a", "b"} };
    auto got = sol.solution(logs, queries);
    check("multi_readd", got, { "[\"b\", \"a\"]", "true" });
}

int main() {
    test_example1();
    test_example2_empty();
    test_multi_readd();
    cout << "\nAll tests passed.\n";
}
