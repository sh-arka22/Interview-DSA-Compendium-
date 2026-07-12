// Your implementation, minimally fixed — structure/approach unchanged.
// Fixes are marked with  // FIX:
#include <vector>
#include <string>
#include <unordered_map>   // FIX: missing include
#include <set>             // FIX: missing include
#include <algorithm>       // FIX: missing include (sort)
#include <functional>      // FIX: needed for recursive lambda
using namespace std;
using ll = long long;      // FIX: ll was used but never defined

class Solution {
    static constexpr ll CASHBACK_DELAY_MS = 86400000LL; // FIX: missing ';'
    static constexpr ll CASHBACK_PCT = 2;               // FIX: was never declared
public:
    vector<string> solution(vector<vector<string>> queries) {
        vector<string> results;
        unordered_map<string, long long> balance;   // FIX: int overflows (1e4 * 1e9 = 1e13)
        unordered_map<string, long long> outgoing;  // FIX: same
        struct Payment {
            string owner;
            long long cashBack;
            bool completed = false;
        };
        unordered_map<string, Payment> payments;
        set<pair<long long, string>> paymentsPending;
        long long payCounter = 0;                   // FIX: payCounter_ was never declared

        // FIX: an 'auto' lambda cannot call itself -> std::function keeps your recursion
        function<void(long long)> processPendingPayments = [&](long long tsp) {
            if (paymentsPending.empty()) return;    // FIX: empty-check BEFORE dereferencing begin()
            auto [t, pid] = *paymentsPending.begin();
            if (t > tsp) return;
            paymentsPending.erase(paymentsPending.begin());
            Payment& p = payments[pid];
            balance[p.owner] += p.cashBack;
            p.completed = true;                     // FIX: member is 'completed', not 'done'
            processPendingPayments(tsp);
        };

        for (auto querie : queries) {
            string cmd = querie[0];
            string tsp = querie[1];
            processPendingPayments(stoll(tsp));

            if (cmd == "CREATE_ACCOUNT") {
                string accId = querie[2];
                if (balance.count(accId)) results.push_back("false");
                else {
                    balance[accId] = 0;
                    outgoing[accId] = 0;
                    results.push_back("true");
                }
            }
            else if (cmd == "DEPOSIT") {
                string accId = querie[2];
                long long amt = stoll(querie[3]);   // FIX: stoll, not stoi
                if (balance.count(accId) == 0) { results.push_back(""); }
                else {
                    balance[accId] += amt;
                    results.push_back(to_string(balance[accId]));
                }
            }
            else if (cmd == "GET_BALANCE") {
                string accId = querie[2];
                if (balance.count(accId) == 0) results.push_back("");
                else {
                    results.push_back(to_string(balance[accId]));
                }
            }
            else if (cmd == "TRANSFER") {
                string src = querie[2];
                string dst = querie[3];
                if (src == dst) {
                    results.push_back("");
                    continue;
                }
                long long amount = stoll(querie[4]); // FIX: stoll + long long
                auto s = balance.find(src), d = balance.find(dst);
                if (s == balance.end() || d == balance.end() || s->second < amount) results.push_back("");
                else {
                    s->second -= amount;
                    d->second += amount;
                    outgoing[src] += amount;
                    results.push_back(to_string(s->second));
                }
            }
            else if (cmd == "TOP_SPENDERS") {
                int n = stoi(querie[2]);
                vector<pair<string, long long>> ranked(outgoing.begin(), outgoing.end());
                // highest total first; ties -> ascending account id
                sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
                    if (a.second != b.second) return a.second > b.second;
                    return a.first < b.first;
                });
                string out;
                for (size_t i = 0; i < ranked.size() && i < (size_t)n; ++i) {
                    if (i) out += ", ";
                    out += ranked[i].first + "(" + to_string(ranked[i].second) + ")";
                }
                results.push_back(out);
            }
            else if (cmd == "PAY") {
                string acc = querie[2];              // FIX: acc was never declared
                long long amount = stoll(querie[3]); // FIX: amount was never declared
                auto it = balance.find(acc);         // FIX: 'balance', not 'balance_'
                if (it == balance.end() || it->second < amount) results.push_back("");
                else {
                    it->second -= amount;
                    outgoing[acc] += amount;
                    string pid = "payment" + to_string(++payCounter);
                    payments[pid] = {acc, amount * CASHBACK_PCT / 100, false};
                    paymentsPending.insert({stoll(tsp) + CASHBACK_DELAY_MS, pid});
                    results.push_back(pid);
                }
            }
            else if (cmd == "GET_PAYMENT_STATUS") {
                string acc = querie[2];              // FIX: acc was never declared
                string pid = querie[3];              // FIX: pid was never declared
                if (!balance.count(acc)) results.push_back("");
                else {
                    auto it = payments.find(pid);    // FIX: 'payments', not 'payments_'
                    if (it == payments.end() || it->second.owner != acc)
                        results.push_back("");       // FIX: was 'return "";' (wrong return type)
                    else                             // FIX: stray ';' inside push_back(...)
                        results.push_back(it->second.completed ? "CASHBACK_RECEIVED" : "IN_PROGRESS");
                }
            }
        }
        return results;
    }
};

// ---------------------------- tests ----------------------------
#include <iostream>
int main() {
    Solution sol;
    vector<vector<string>> q = {
        {"CREATE_ACCOUNT", "1", "alice"},
        {"CREATE_ACCOUNT", "2", "bob"},
        {"DEPOSIT", "3", "alice", "2000"},
        {"TRANSFER", "4", "alice", "bob", "500"},
        {"TOP_SPENDERS", "5", "2"},
        {"PAY", "6", "alice", "1000"},
        {"GET_PAYMENT_STATUS", "7", "alice", "payment1"},
        {"GET_PAYMENT_STATUS", "86400007", "alice", "payment1"},
        {"GET_BALANCE", "86400008", "alice"},
    };
    vector<string> expected = {"true", "true", "2000", "1500", "alice(500), bob(0)",
                               "payment1", "IN_PROGRESS", "CASHBACK_RECEIVED", "520"};
    auto got = sol.solution(q);
    if (got != expected) {
        for (size_t i = 0; i < expected.size(); ++i)
            if (got[i] != expected[i])
                cerr << "[" << i << "] got \"" << got[i] << "\" expected \"" << expected[i] << "\"\n";
        return 1;
    }
    cout << "All tests passed.\n";
    return 0;
}
