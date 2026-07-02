// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <iostream>
using namespace std;

Solution sol;

static void check(const string& name,
                  vector<vector<string>> cmds,
                  vector<string> exp) {
    vector<string> got = sol.solution(cmds);
    if (got != exp) {
        cerr << "FAIL  " << name << "\n  got: [";
        for (auto& s : got) cerr << s << " ";
        cerr << "]\n  exp: [";
        for (auto& s : exp) cerr << s << " ";
        cerr << "]\n";
        exit(1);
    }
    cout << "PASS  " << name << "\n";
}

int main() {
    // Example 1: nested BEGINs, a DELETE inside the inner txn, ROLLBACK restores the
    // enclosing value 20, then COMMIT flattens it into the base.
    check("ex1: nested rollback then commit",
        {{"SET","a","10"},{"GET","a"},{"BEGIN"},{"SET","a","20"},{"BEGIN"},
         {"DELETE","a"},{"GET","a"},{"ROLLBACK"},{"GET","a"},{"COMMIT"},{"GET","a"}},
        {"10","NULL","20","20"});

    // Example 2: COMMIT with nothing open.
    check("ex2: commit no txn", {{"COMMIT"}}, {"NO TRANSACTION"});

    // ROLLBACK with nothing open.
    check("rollback no txn", {{"ROLLBACK"}}, {"NO TRANSACTION"});

    // COUNT stays correct as values move between keys and a key is deleted. O(1) each.
    check("count is incremental",
        {{"SET","a","1"},{"SET","b","1"},{"COUNT","1"},   // 2 keys hold "1"
         {"SET","b","2"},{"COUNT","1"},{"COUNT","2"},      // b left "1" -> counts 1 and 1
         {"DELETE","a"},{"COUNT","1"}},                    // "1" now held by nobody -> 0
        {"2","1","1","0"});

    // DELETE of a missing key is a pure no-op (no journal, no crash); GET stays NULL.
    check("delete missing is no-op",
        {{"DELETE","x"},{"GET","x"},{"SET","x","5"},{"DELETE","x"},{"GET","x"}},
        {"NULL","NULL"});

    // Deep nesting: two ROLLBACKs peel back one level each, restoring 3 -> 2 -> 1.
    check("nested rollback peels one level",
        {{"SET","a","1"},{"BEGIN"},{"SET","a","2"},{"BEGIN"},{"SET","a","3"},
         {"GET","a"},{"ROLLBACK"},{"GET","a"},{"ROLLBACK"},{"GET","a"}},
        {"3","2","1"});

    // Same key written twice in one txn: reverse-order undo replay must land on the
    // ORIGINAL (absent) state, not the intermediate "1".
    check("double write then rollback (reverse order)",
        {{"BEGIN"},{"SET","a","1"},{"SET","a","2"},{"GET","a"},{"ROLLBACK"},{"GET","a"}},
        {"2","NULL"});

    // COMMIT flattens every open level at once; a following ROLLBACK finds nothing open.
    check("commit flattens all levels",
        {{"BEGIN"},{"SET","a","1"},{"BEGIN"},{"SET","b","1"},{"COMMIT"},
         {"GET","a"},{"GET","b"},{"COUNT","1"},{"ROLLBACK"}},
        {"1","1","2","NO TRANSACTION"});

    cout << "\nAll tests passed.\n";
}
