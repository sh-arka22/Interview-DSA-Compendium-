#include <vector>
#include <string>
#include <list>
#include <unordered_map>
using namespace std;

class Solution {
    static vector<string> splitCSV(const string& s) {
        vector<string> out;
        string cur;
        for (char c : s) {
            if (c == ',') { out.push_back(cur); cur.clear(); }
            else cur += c;
        }
        out.push_back(cur);
        return out;
    }

    static string listToStr(const vector<string>& v) {
        string s = "[";
        for (size_t i = 0; i < v.size(); i++) {
            if (i) s += ", ";
            s += "\"" + v[i] + "\"";
        }
        return s + "]";
    }

    struct Entry { string user; long long key; };

    // Part 1 kept exactly one {order, pos, counter} triple for the whole system.
    // Part 2's ONLY change: namespace that triple per community, in an outer hash map.
    // Every operation now costs one extra hash lookup to find the right community
    // (or falls through to the "empty / false" default when it has never been seen).
    struct Community {
        list<Entry> order;                                  // head..tail = highest..lowest rank
        unordered_map<string, list<Entry>::iterator> pos;    // user -> node, O(1) lookup
        long long counter = 0;
    };

public:
    // logs:    "<community>,<target>,<action>,<actor>,<timestamp>"
    // queries: ["getModRanking", community]                -> "[user1, user2, ...]"
    //          ["canRemoveMod", community, target, actor]  -> "true" / "false"
    vector<string> solution(vector<string>& logs, vector<vector<string>>& queries) {
        unordered_map<string, Community> communities;

        // ---- replay the log to build current per-community state ----
        for (auto& log : logs) {
            auto f = splitCSV(log);
            const string& community = f[0];
            const string& target    = f[1];
            const string& action    = f[2];
            Community& c = communities[community];
            if (action == "added") {
                c.order.push_back({target, c.counter++});
                c.pos[target] = prev(c.order.end());
            } else {                                          // "removed"
                c.order.erase(c.pos[target]);
                c.pos.erase(target);
            }
        }

        // ---- answer each query in order ----
        vector<string> results;
        results.reserve(queries.size());

        for (auto& q : queries) {
            const string& method = q[0];

            if (method == "getModRanking") {
                const string& community = q[1];
                vector<string> ranking;
                auto ci = communities.find(community);
                if (ci != communities.end())
                    for (auto& e : ci->second.order) ranking.push_back(e.user);
                results.push_back(listToStr(ranking));
            }
            else { // "canRemoveMod"
                const string& community = q[1];
                const string& target    = q[2];
                const string& actor     = q[3];
                bool ok = false;
                if (target != actor) {
                    auto ci = communities.find(community);
                    if (ci != communities.end()) {
                        auto& pos = ci->second.pos;
                        auto ti = pos.find(target), ai = pos.find(actor);
                        if (ti != pos.end() && ai != pos.end())
                            ok = ai->second->key < ti->second->key;
                    }
                }
                results.push_back(ok ? "true" : "false");
            }
        }

        return results;
    }
};
