#include <vector>
#include <string>
#include <list>
#include <unordered_map>
using namespace std;

class Solution {
    // Split a comma-separated record into fields (fields themselves never contain commas).
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

    // Render a ranking list as ["a", "b"].
    static string listToStr(const vector<string>& v) {
        string s = "[";
        for (size_t i = 0; i < v.size(); i++) {
            if (i) s += ", ";
            s += "\"" + v[i] + "\"";
        }
        return s + "]";
    }

    // key: append-order counter, smaller = promoted earlier = higher rank.
    struct Entry { string user; long long key; };

public:
    // KEY INSIGHT: logs arrive in strictly increasing timestamp order, and we process them
    // in that same order, so "processing order" IS "chronological order." A freshly-added
    // moderator therefore always has the newest promotion time of anyone currently active,
    // so it always belongs at the TAIL (lowest rank) -- push_back, never insert-and-sort.
    // A plain append-order counter is used as a stand-in for the timestamp: it reproduces
    // the exact same relative order without ever having to parse or store one.
    //
    // Structure: doubly linked list (rank order, head=highest..tail=lowest) + hash map
    // user -> list iterator, giving O(1) add / remove / rank-compare and O(k) full listing.
    //
    // logs:    "<target>,<action>,<actor>,<timestamp>"      (action: "added" / "removed")
    // queries: ["getModRanking"]                 -> "[user1, user2, ...]" highest -> lowest
    //          ["canRemoveMod", target, actor]    -> "true" / "false"
    vector<string> solution(vector<string>& logs, vector<vector<string>>& queries) {
        list<Entry> order;                                   // head..tail = highest..lowest rank
        unordered_map<string, list<Entry>::iterator> pos;     // user -> node, O(1) lookup
        long long counter = 0;

        // ---- replay the log to build current moderator state ----
        for (auto& log : logs) {
            auto f = splitCSV(log);
            const string& target = f[0];
            const string& action = f[1];
            if (action == "added") {
                order.push_back({target, counter++});
                pos[target] = prev(order.end());
            } else {                                          // "removed"
                order.erase(pos[target]);
                pos.erase(target);
            }
        }

        // ---- answer each query in order ----
        vector<string> results;
        results.reserve(queries.size());

        for (auto& q : queries) {
            const string& method = q[0];

            if (method == "getModRanking") {
                vector<string> ranking;
                ranking.reserve(order.size());
                for (auto& e : order) ranking.push_back(e.user);
                results.push_back(listToStr(ranking));
            }
            else { // "canRemoveMod"
                const string& target = q[1];
                const string& actor  = q[2];
                bool ok = false;
                if (target != actor) {
                    auto ti = pos.find(target), ai = pos.find(actor);
                    if (ti != pos.end() && ai != pos.end())
                        ok = ai->second->key < ti->second->key;   // actor promoted earlier?
                }
                results.push_back(ok ? "true" : "false");
            }
        }

        return results;
    }
};
