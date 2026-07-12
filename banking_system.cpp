// ============================================================================
// Banking System Simulation (CodeSignal-style, Levels 1-4) — C++17
// ----------------------------------------------------------------------------
// Design (first principles):
//   balance[acc], outgoing[acc]        : core state (int64 — sums can exceed 2^31)
//   payments[pid] = {owner, cb, done}  : owner = CURRENT owner (survives merges)
//   accPayments[acc] = [pid...]        : for O(k) ownership redirect on merge
//   cashbackHeap (min-heap by time)    : pending cashbacks, stores payment ids
//
// Key ideas:
//   1. LAZY SETTLEMENT — before every query, pop all heap entries with
//      creditTime <= ts and credit 2% to payments[pid].owner.
//   2. Heap stores PAYMENT IDS, not accounts — MERGE only re-points
//      payment ownership; pending cashbacks & status follow automatically.
//
// Complexity: n <= 1e4 queries. Each op O(log n) amortized,
//   TOP_SPENDERS O(A log A), MERGE O(#payments of merged account).
// ============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <utility>
using namespace std;
using ll = long long;

class BankingSystem {
    static constexpr ll CASHBACK_DELAY_MS = 86400000LL; // 24h
    static constexpr ll CASHBACK_PCT = 2;

    struct Payment {
        string owner;   // current owner (updated on merge)
        ll cashback;
        bool done = false;
    };

    unordered_map<string, ll> balance_;
    unordered_map<string, ll> outgoing_;
    unordered_map<string, Payment> payments_;             // pid -> payment
    unordered_map<string, vector<string>> accPayments_;   // acc -> [pid]
    // min-heap of (creditTime, pid)
    priority_queue<pair<ll, string>, vector<pair<ll, string>>, greater<>> cashbackHeap_;
    ll payCounter_ = 0;

public:
    // ------------------------------------------------------------------
    // Lazy cashback settlement — MUST run before every operation.
    // ------------------------------------------------------------------
    void settle(ll ts) {
        while (!cashbackHeap_.empty() && cashbackHeap_.top().first <= ts) {
            auto [t, pid] = cashbackHeap_.top();
            cashbackHeap_.pop();
            Payment& p = payments_[pid];
            balance_[p.owner] += p.cashback;
            p.done = true;
        }
    }

    // ==================================================================
    // Level 1 — Accounts and balances
    // ==================================================================
    string createAccount(ll ts, const string& acc) {
        if (balance_.count(acc)) return "false";
        balance_[acc] = 0;
        outgoing_[acc] = 0;
        accPayments_[acc] = {};
        return "true";
    }

    string deposit(ll ts, const string& acc, ll amount) {
        auto it = balance_.find(acc);
        if (it == balance_.end()) return "";
        it->second += amount;
        return to_string(it->second);
    }

    string getBalance(ll ts, const string& acc) {
        auto it = balance_.find(acc);
        if (it == balance_.end()) return "";
        return to_string(it->second);
    }

    // ==================================================================
    // Level 2 — Transfers and spending ranks
    // ==================================================================
    string transfer(ll ts, const string& src, const string& dst, ll amount) {
        if (src == dst) return "";
        auto s = balance_.find(src), d = balance_.find(dst);
        if (s == balance_.end() || d == balance_.end() || s->second < amount) return "";
        s->second -= amount;
        d->second += amount;
        outgoing_[src] += amount;
        return to_string(s->second);
    }

    string topSpenders(ll ts, ll n) {
        vector<pair<string, ll>> ranked(outgoing_.begin(), outgoing_.end());
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
        return out;
    }

    // ==================================================================
    // Level 3 — Payments with deferred cashback
    // ==================================================================
    string pay(ll ts, const string& acc, ll amount) {
        auto it = balance_.find(acc);
        if (it == balance_.end() || it->second < amount) return "";
        it->second -= amount;
        outgoing_[acc] += amount;
        string pid = "payment" + to_string(++payCounter_);
        payments_[pid] = {acc, amount * CASHBACK_PCT / 100, false};
        accPayments_[acc].push_back(pid);
        cashbackHeap_.emplace(ts + CASHBACK_DELAY_MS, pid);
        return pid;
    }

    string getPaymentStatus(ll ts, const string& acc, const string& pid) {
        if (!balance_.count(acc)) return "";
        auto it = payments_.find(pid);
        if (it == payments_.end() || it->second.owner != acc) return "";
        return it->second.done ? "CASHBACK_RECEIVED" : "IN_PROGRESS";
    }

    // ==================================================================
    // Level 4 — Account merge
    // ==================================================================
    string mergeAccounts(ll ts, const string& a1, const string& a2) {
        if (a1 == a2 || !balance_.count(a1) || !balance_.count(a2)) return "false";
        balance_[a1] += balance_[a2];
        outgoing_[a1] += outgoing_[a2];
        // Redirect payment ownership: pending cashbacks & status follow.
        auto& moved = accPayments_[a2];
        for (const string& pid : moved) payments_[pid].owner = a1;
        auto& dst = accPayments_[a1];
        dst.insert(dst.end(), moved.begin(), moved.end());
        balance_.erase(a2);
        outgoing_.erase(a2);
        accPayments_.erase(a2);
        return "true";
    }
};

// ============================================================================
// Dispatcher — one result string per query
// ============================================================================
vector<string> solution(const vector<vector<string>>& queries) {
    BankingSystem bank;
    vector<string> results;
    results.reserve(queries.size());

    for (const auto& q : queries) {
        const string& op = q[0];
        ll ts = stoll(q[1]);
        bank.settle(ts); // apply all cashbacks due at/before ts

        if      (op == "CREATE_ACCOUNT")     results.push_back(bank.createAccount(ts, q[2]));
        else if (op == "DEPOSIT")            results.push_back(bank.deposit(ts, q[2], stoll(q[3])));
        else if (op == "GET_BALANCE")        results.push_back(bank.getBalance(ts, q[2]));
        else if (op == "TRANSFER")           results.push_back(bank.transfer(ts, q[2], q[3], stoll(q[4])));
        else if (op == "TOP_SPENDERS")       results.push_back(bank.topSpenders(ts, stoll(q[2])));
        else if (op == "PAY")                results.push_back(bank.pay(ts, q[2], stoll(q[3])));
        else if (op == "GET_PAYMENT_STATUS") results.push_back(bank.getPaymentStatus(ts, q[2], q[3]));
        else if (op == "MERGE_ACCOUNTS")     results.push_back(bank.mergeAccounts(ts, q[2], q[3]));
        else                                 results.push_back("");
    }
    return results;
}

// ============================================================================
// Tests
// ============================================================================
static void check(const vector<vector<string>>& q, const vector<string>& expected,
                  const string& name) {
    vector<string> got = solution(q);
    if (got != expected) {
        cerr << "FAIL: " << name << "\n";
        for (size_t i = 0; i < expected.size(); ++i)
            if (i >= got.size() || got[i] != expected[i])
                cerr << "  [" << i << "] got \"" << (i < got.size() ? got[i] : "<none>")
                     << "\" expected \"" << expected[i] << "\"\n";
        exit(1);
    }
    cout << "PASS: " << name << "\n";
}

int main() {
    // Provided example
    check({
        {"CREATE_ACCOUNT", "1", "alice"},
        {"CREATE_ACCOUNT", "2", "bob"},
        {"DEPOSIT", "3", "alice", "2000"},
        {"TRANSFER", "4", "alice", "bob", "500"},
        {"TOP_SPENDERS", "5", "2"},
        {"PAY", "6", "alice", "1000"},
        {"GET_PAYMENT_STATUS", "7", "alice", "payment1"},
        {"GET_PAYMENT_STATUS", "86400007", "alice", "payment1"},
        {"GET_BALANCE", "86400008", "alice"},
    }, {
        "true", "true", "2000", "1500", "alice(500), bob(0)",
        "payment1", "IN_PROGRESS", "CASHBACK_RECEIVED", "520",
    }, "example");

    // Merge: pending cashback redirected, status queryable via new owner
    check({
        {"CREATE_ACCOUNT", "1", "a"},
        {"CREATE_ACCOUNT", "2", "b"},
        {"DEPOSIT", "3", "b", "1000"},
        {"PAY", "4", "b", "500"},                     // payment1, cb 10 at t=86400004
        {"MERGE_ACCOUNTS", "5", "a", "b"},            // b folded into a
        {"GET_BALANCE", "6", "b"},                    // "" (gone)
        {"GET_PAYMENT_STATUS", "7", "a", "payment1"}, // IN_PROGRESS via a
        {"GET_BALANCE", "86400004", "a"},             // 500 + 10 cashback
        {"GET_PAYMENT_STATUS", "86400005", "a", "payment1"},
        {"TOP_SPENDERS", "86400006", "5"},            // a inherits b's outgoing
    }, {
        "true", "true", "1000", "payment1", "true", "",
        "IN_PROGRESS", "510", "CASHBACK_RECEIVED", "a(500)",
    }, "merge + cashback redirect");

    // Invalid ops
    check({
        {"CREATE_ACCOUNT", "1", "x"},
        {"CREATE_ACCOUNT", "2", "x"},          // false (duplicate)
        {"DEPOSIT", "3", "ghost", "100"},      // ""
        {"TRANSFER", "4", "x", "x", "0"},      // "" (self)
        {"TRANSFER", "5", "x", "ghost", "10"}, // "" (missing target)
        {"PAY", "6", "x", "1"},                // "" (insufficient)
        {"MERGE_ACCOUNTS", "7", "x", "x"},     // false
        {"GET_PAYMENT_STATUS", "8", "x", "payment1"}, // "" (no such payment)
    }, {
        "true", "false", "", "", "", "", "false", "",
    }, "invalid operations");

    cout << "All tests passed.\n";
    return 0;
}
