// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <cassert>
#include <iostream>
#include <random>
#include <algorithm>
using namespace std;

static void check(const string& name, bool cond) {
    if (!cond) { cerr << "FAIL  " << name << "\n"; exit(1); }
    cout << "PASS  " << name << "\n";
}

static bool eq(const vector<string>& a, const vector<string>& b) {
    return a == b;
}

Solution sol;

void test_example1() {
    vector<vector<string>> ops = {
        {"ADD_GRANT","0","100","10"},
        {"ADD_GRANT","1","50","5"},
        {"GET_BALANCE","2"},
        {"SPEND","3","60"},
        {"GET_BALANCE","4"},
        {"GET_BALANCE","6"}
    };
    vector<string> expected = {"150","90","90"};
    check("example1", eq(sol.solution(ops), expected));
}

void test_example2_half_open_boundary() {
    vector<vector<string>> ops = {
        {"ADD_GRANT","0","100","10"},
        {"GET_BALANCE","10"}
    };
    vector<string> expected = {"0"};
    check("example2: half-open boundary", eq(sol.solution(ops), expected));
}

void test_spend_more_than_balance() {
    vector<vector<string>> ops = {
        {"ADD_GRANT","0","30","100"},
        {"SPEND","1","1000"},        // way more than available
        {"GET_BALANCE","2"}
    };
    vector<string> expected = {"0"};
    check("spend exceeds balance -> floors at 0", eq(sol.solution(ops), expected));
}

void test_spend_prefers_earliest_expiry_regardless_of_add_order() {
    // Long-lived grant added first, short-lived grant added second.
    // SPEND must still drain the short-lived (soonest expiry) one first.
    vector<vector<string>> ops = {
        {"ADD_GRANT","0","10","100"},   // expires 100
        {"ADD_GRANT","1","5","3"},      // expires 3 (soonest)
        {"SPEND","2","3"},
        {"GET_BALANCE","2"},
        {"GET_BALANCE","4"}             // grant expiring at 3 is gone now
    };
    // After spend: soonest-expiry grant (5, expire 3) reduced by 3 -> 2 left.
    // total at t=2: 2 (from expire-3 grant) + 10 (expire-100 grant) = 12
    // at t=4: expire-3 grant is gone (3<=4), so only 10 remains
    vector<string> expected = {"12","10"};
    check("spend drains soonest-expiry grant first", eq(sol.solution(ops), expected));
}

void test_multiple_partial_spends_across_grants() {
    vector<vector<string>> ops = {
        {"ADD_GRANT","0","20","5"},
        {"ADD_GRANT","0","20","10"},
        {"ADD_GRANT","0","20","15"},
        {"SPEND","1","25"},       // drains grant@5 (20) + 5 from grant@10
        {"GET_BALANCE","1"},      // 15 (grant@10) + 20 (grant@15) = 35
        {"SPEND","6","10"},       // grant@5 already gone; take from grant@10 (15->5)
        {"GET_BALANCE","6"}       // 5 + 20 = 25
    };
    vector<string> expected = {"35","25"};
    check("multiple partial spends across grants", eq(sol.solution(ops), expected));
}

void test_no_grants_spend_and_balance_are_zero() {
    vector<vector<string>> ops = {
        {"SPEND","0","10"},
        {"GET_BALANCE","0"}
    };
    vector<string> expected = {"0"};
    check("no grants at all -> 0", eq(sol.solution(ops), expected));
}

void test_exact_drain_to_zero() {
    vector<vector<string>> ops = {
        {"ADD_GRANT","0","50","20"},
        {"SPEND","1","50"},
        {"GET_BALANCE","1"}
    };
    vector<string> expected = {"0"};
    check("exact drain leaves 0, not negative", eq(sol.solution(ops), expected));
}

// ---- Brute-force reference: O(ops^2), rescans + sorts every time ----
static vector<string> bruteForce(const vector<vector<string>>& ops) {
    vector<pair<long long,long long>> grants; // (expire, amount)
    vector<string> result;
    for (auto& op : ops) {
        const string& type = op[0];
        long long ts = stoll(op[1]);
        // drop expired
        grants.erase(remove_if(grants.begin(), grants.end(),
            [&](const pair<long long,long long>& g) { return g.first <= ts; }),
            grants.end());
        if (type == "ADD_GRANT") {
            long long amount = stoll(op[2]);
            long long expire = stoll(op[3]);
            grants.push_back({expire, amount});
        } else if (type == "SPEND") {
            sort(grants.begin(), grants.end());
            long long remaining = stoll(op[2]);
            for (auto& g : grants) {
                if (remaining == 0) break;
                long long take = min(g.second, remaining);
                g.second -= take;
                remaining -= take;
            }
            grants.erase(remove_if(grants.begin(), grants.end(),
                [](const pair<long long,long long>& g) { return g.second == 0; }),
                grants.end());
        } else {
            long long total = 0;
            for (auto& g : grants) total += g.second;
            result.push_back(to_string(total));
        }
    }
    return result;
}

void test_random_cross_check() {
    mt19937 rng(7);
    uniform_int_distribution<int> opType(0, 2);       // 0=ADD,1=SPEND,2=GET
    uniform_int_distribution<int> amtDist(1, 20);
    uniform_int_distribution<int> offDist(1, 15);
    uniform_int_distribution<int> tickDist(0, 3);      // timestamp advance

    for (int trial = 0; trial < 200; trial++) {
        int n = 5 + trial % 40;
        long long t = 0;
        vector<vector<string>> ops;
        for (int i = 0; i < n; i++) {
            t += tickDist(rng);
            int kind = opType(rng);
            if (kind == 0) {
                long long amount = amtDist(rng);
                long long expire = t + offDist(rng);
                ops.push_back({"ADD_GRANT", to_string(t), to_string(amount), to_string(expire)});
            } else if (kind == 1) {
                long long amount = amtDist(rng);
                ops.push_back({"SPEND", to_string(t), to_string(amount)});
            } else {
                ops.push_back({"GET_BALANCE", to_string(t)});
            }
        }
        auto expected = bruteForce(ops);
        auto got = sol.solution(ops);
        if (got != expected) {
            cerr << "FAIL  random trial " << trial << "\n";
            for (auto& op : ops) {
                for (auto& s : op) cerr << s << " ";
                cerr << "| ";
            }
            cerr << "\nexpected: "; for (auto& s : expected) cerr << s << " ";
            cerr << "\ngot:      "; for (auto& s : got) cerr << s << " ";
            cerr << "\n";
            exit(1);
        }
    }
    cout << "PASS  random cross-check vs brute force (200 trials)\n";
}

int main() {
    test_example1();
    test_example2_half_open_boundary();
    test_spend_more_than_balance();
    test_spend_prefers_earliest_expiry_regardless_of_add_order();
    test_multiple_partial_spends_across_grants();
    test_no_grants_spend_and_balance_are_zero();
    test_exact_drain_to_zero();
    test_random_cross_check();
    cout << "\nAll tests passed.\n";
}
