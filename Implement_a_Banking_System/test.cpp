// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <cassert>
#include <iostream>
using namespace std;

static void check(const string& name, bool cond) {
    if (!cond) { cerr << "FAIL  " << name << "\n"; exit(1); }
    cout << "PASS  " << name << "\n";
}

Solution sol;

// Example 1: basic create/deposit/top_outgoing
void test_example1() {
    auto r = sol.solution({
        {"create_account", "1", "alice"},
        {"create_account", "2", "alice"},
        {"deposit", "3", "bob", "50"},
        {"create_account", "4", "bob"},
        {"deposit", "5", "alice", "100"},
        {"deposit", "6", "bob", "100"},
        {"top_outgoing", "7", "3"},
        {"top_outgoing", "8", "0"},
    });
    check("ex1: create ok",     r[0] == "true");
    check("ex1: dup create",    r[1] == "false");
    check("ex1: missing acct",  r[2] == "null");
    check("ex1: create bob",    r[3] == "true");
    check("ex1: deposit alice", r[4] == "100");
    check("ex1: deposit bob",   r[5] == "100");
    check("ex1: top 3",         r[6] == "[\"alice\", \"bob\"]");
    check("ex1: top 0",         r[7] == "[]");
}

// Example 2: transfer lifecycle (create, pending, accept, double-accept)
void test_example2() {
    auto r = sol.solution({
        {"create_account", "1", "A"},
        {"create_account", "2", "B"},
        {"deposit", "3", "A", "500"},
        {"transfer", "4", "A", "B", "200"},
        {"top_outgoing", "5", "2"},
        {"accept_transfer", "6", "A", "transfer1"},
        {"accept_transfer", "7", "B", "transfer1"},
        {"top_outgoing", "8", "2"},
        {"deposit", "9", "B", "50"},
        {"accept_transfer", "10", "B", "transfer1"},
    });
    check("ex2: create A",      r[0] == "true");
    check("ex2: create B",      r[1] == "true");
    check("ex2: deposit 500",   r[2] == "500");
    check("ex2: transfer",      r[3] == "transfer1");
    check("ex2: top before",    r[4] == "[\"A\", \"B\"]");
    check("ex2: wrong target",  r[5] == "false");
    check("ex2: accept ok",     r[6] == "true");
    check("ex2: top after",     r[7] == "[\"A\", \"B\"]");
    check("ex2: deposit B",     r[8] == "250");
    check("ex2: dup accept",    r[9] == "false");
}

// Expired transfer returns amount to source before next deposit
void test_expiration() {
    auto r = sol.solution({
        {"create_account", "1", "A"},
        {"create_account", "2", "B"},
        {"deposit", "3", "A", "1000"},
        {"transfer", "4", "A", "B", "300"},
        // t=86400005 > 4+86400000 → transfer expires, 300 back to A
        {"deposit", "86400005", "A", "0"},
    });
    // A: 1000 - 300 (withheld) + 300 (expired back) + 0 = 1000
    check("expire: balance restored", r[4] == "1000");
}

// Invalid transfers: insufficient balance, same account, missing account
void test_transfer_invalid() {
    auto r = sol.solution({
        {"create_account", "1", "A"},
        {"create_account", "2", "B"},
        {"deposit", "3", "A", "100"},
        {"transfer", "4", "A", "B", "200"},
        {"transfer", "5", "A", "A", "50"},
        {"transfer", "6", "A", "C", "50"},
    });
    check("invalid: insufficient",   r[3] == "null");
    check("invalid: same account",   r[4] == "null");
    check("invalid: missing target", r[5] == "null");
}

// merge_accounts: balances combined, direct mutual transfer cancelled
void test_merge_cancel() {
    auto r = sol.solution({
        {"create_account", "1", "A"},
        {"create_account", "2", "B"},
        {"deposit", "3", "A", "100"},
        {"deposit", "4", "B", "50"},
        {"transfer", "5", "A", "B", "30"},
        {"merge_accounts", "6", "B", "A"},
        // B: 50 (B balance) + 70 (A available) + 30 (cancelled A→B) = 150
        {"deposit", "7", "B", "0"},
    });
    check("merge: returns true", r[5] == "true");
    check("merge: B balance",    r[6] == "150");
}

// merge_accounts: pending outgoing from src redirected to dest
void test_merge_redirect() {
    auto r = sol.solution({
        {"create_account", "1", "A"},
        {"create_account", "2", "B"},
        {"create_account", "3", "C"},
        {"deposit", "4", "A", "200"},
        {"transfer", "5", "A", "C", "80"},
        {"merge_accounts", "6", "B", "A"},  // A→C becomes B→C
        {"accept_transfer", "7", "C", "transfer1"},
        {"top_outgoing", "8", "2"},
    });
    check("redirect: merge ok",  r[5] == "true");
    check("redirect: accept ok", r[6] == "true");
    check("redirect: B top",     r[7] == "[\"B\", \"C\"]");
}

// top_outgoing: descending by outgoing, lexicographic tie-break
void test_top_ordering() {
    auto r = sol.solution({
        {"create_account", "1", "Z"},
        {"create_account", "2", "A"},
        {"create_account", "3", "M"},
        {"deposit", "4", "Z", "100"},
        {"transfer", "5", "Z", "M", "100"},
        {"accept_transfer", "6", "M", "transfer1"},
        {"top_outgoing", "7", "3"},
    });
    // Z outgoing=100, A outgoing=0, M outgoing=0 → Z first, then A < M
    check("top order", r[6] == "[\"Z\", \"A\", \"M\"]");
}

int main() {
    test_example1();
    test_example2();
    test_expiration();
    test_transfer_invalid();
    test_merge_cancel();
    test_merge_redirect();
    test_top_ordering();
    cout << "\nAll tests passed.\n";
}
