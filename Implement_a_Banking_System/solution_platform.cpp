#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
using namespace std;

class Solution {
    static constexpr long long EXPIRATION = 86400000LL;

    struct Account {
        long long balance  = 0;
        long long outgoing = 0;
    };

    struct Transfer {
        string    source;
        string    target;
        long long amount;
        long long createdAt;
        bool      active = true;
    };

    unordered_map<string, Account> accounts;
    vector<Transfer>               transfers;

    // Manual digit parser: handles "7", "7.0", "100.0" — no library calls, never throws
    static long long toLL(const string& s) {
        long long v = 0;
        int i = 0, n = (int)s.size();
        while (i < n && s[i] == ' ') i++;
        int sign = 1;
        if (i < n && s[i] == '-') { sign = -1; i++; }
        else if (i < n && s[i] == '+') i++;
        while (i < n && s[i] >= '0' && s[i] <= '9')
            v = v * 10 + (s[i++] - '0');
        return sign * v;
    }
    static int toI(const string& s) { return (int)toLL(s); }

    void expirePending(long long ts) {
        for (int i = 0; i < (int)transfers.size(); i++) {
            Transfer& t = transfers[i];
            if (t.active && t.createdAt + EXPIRATION < ts) {
                t.active = false;
                auto it = accounts.find(t.source);
                if (it != accounts.end())
                    it->second.balance += t.amount;
            }
        }
    }

    string listToStr(const vector<string>& v) {
        string s = "[";
        for (int i = 0; i < (int)v.size(); i++) {
            if (i) s += ", ";
            s += "\"" + v[i] + "\"";
        }
        return s + "]";
    }

public:
    vector<string> solution(vector<vector<string>> operations) {
        vector<string> results;
        accounts.clear();
        transfers.clear();

        for (int oi = 0; oi < (int)operations.size(); oi++) {
            vector<string>& op = operations[oi];
            const string& cmd = op[0];
            long long ts = toLL(op[1]);

            if (cmd == "create_account") {
                const string& id = op[2];
                if (accounts.count(id)) {
                    results.push_back("false");
                } else {
                    accounts[id] = Account();
                    results.push_back("true");
                }
            }
            else if (cmd == "deposit") {
                expirePending(ts);
                const string& id = op[2];
                long long amt = toLL(op[3]);
                if (!accounts.count(id)) {
                    results.push_back("null");
                } else {
                    accounts[id].balance += amt;
                    results.push_back(to_string(accounts[id].balance));
                }
            }
            else if (cmd == "top_outgoing") {
                expirePending(ts);
                int n = toI(op[2]);
                vector<pair<long long, string>> v;
                // avoid C++17 structured bindings
                for (auto it = accounts.begin(); it != accounts.end(); ++it)
                    v.push_back(make_pair(it->second.outgoing, it->first));
                sort(v.begin(), v.end(), [](const pair<long long,string>& a,
                                            const pair<long long,string>& b) {
                    return a.first != b.first ? a.first > b.first : a.second < b.second;
                });
                vector<string> topN;
                int lim = n < (int)v.size() ? n : (int)v.size();
                for (int i = 0; i < lim; i++) topN.push_back(v[i].second);
                results.push_back(listToStr(topN));
            }
            else if (cmd == "transfer") {
                expirePending(ts);
                const string& src = op[2];
                const string& tgt = op[3];
                long long amt = toLL(op[4]);
                if (!accounts.count(src) || !accounts.count(tgt)
                    || src == tgt || accounts[src].balance < amt) {
                    results.push_back("null");
                } else {
                    accounts[src].balance -= amt;
                    Transfer tr;
                    tr.source = src; tr.target = tgt;
                    tr.amount = amt; tr.createdAt = ts; tr.active = true;
                    transfers.push_back(tr);
                    results.push_back("transfer" + to_string(transfers.size()));
                }
            }
            else if (cmd == "accept_transfer") {
                expirePending(ts);
                const string& id = op[2];
                const string& tid = op[3];
                int idx = toI(tid.size() > 8 ? tid.substr(8) : "0") - 1;
                if (idx < 0 || idx >= (int)transfers.size()
                    || !transfers[idx].active || transfers[idx].target != id) {
                    results.push_back("false");
                } else {
                    Transfer& t = transfers[idx];
                    t.active = false;
                    accounts[t.target].balance  += t.amount;
                    accounts[t.source].outgoing += t.amount;
                    results.push_back("true");
                }
            }
            else if (cmd == "merge_accounts") {
                expirePending(ts);
                const string& dest = op[2];
                const string& src  = op[3];
                if (!accounts.count(dest) || !accounts.count(src) || dest == src) {
                    results.push_back("false");
                } else {
                    accounts[dest].balance  += accounts[src].balance;
                    accounts[dest].outgoing += accounts[src].outgoing;
                    for (int i = 0; i < (int)transfers.size(); i++) {
                        Transfer& t = transfers[i];
                        if (!t.active) continue;
                        bool fSrc = t.source == src,  fDst = t.source == dest;
                        bool tSrc = t.target == src,  tDst = t.target == dest;
                        if ((fSrc && tDst) || (fDst && tSrc)) {
                            t.active = false;
                            accounts[dest].balance += t.amount;
                        } else {
                            if (fSrc) t.source = dest;
                            if (tSrc) t.target = dest;
                        }
                    }
                    accounts.erase(src);
                    results.push_back("true");
                }
            }
        }
        return results;
    }
};
