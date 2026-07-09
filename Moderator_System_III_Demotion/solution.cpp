#include <vector>
#include <string>
#include <list>
#include <unordered_map>
#include <algorithm>
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

    // `key` still exists exactly as in Part 1/2 (append-order counter, smaller = higher
    // rank) -- Part 3 just stops treating it as immutable: demote() is allowed to swap
    // it between two adjacent nodes.
    struct Entry { string user; long long key; };
    struct Community {
        list<Entry> order;                                  // head..tail = highest..lowest rank
        unordered_map<string, list<Entry>::iterator> pos;    // user -> node, O(1) lookup
        long long counter = 0;
    };

public:
    // logs:    "<community>,<target>,<action>,<actor>,<timestamp>"    (unchanged from Part 2)
    // queries: ["getModRanking", community]                -> "[user1, user2, ...]"
    //          ["canRemoveMod", community, target, actor]  -> "true" / "false"
    //          ["demote", community, user]                 -> "null" (the spec's `None`;
    //                                                          kept so results.size()==queries.size())
    //
    // NEW IN PART 3 -- demote(community, user): move `user` down exactly one rank.
    // No-op if `user` isn't a moderator there, or is already lowest-ranked.
    //
    // The ranking is no longer *purely* a timestamp sort once a demotion happens, so the
    // structure has to separate "the order" from "the reason for the order" and let the
    // order be nudged directly. Demoting is swapping two ADJACENT list nodes:
    //   1. Physical move  -- std::list::splice relinks in O(1), no allocation, no other
    //      iterator invalidated.
    //   2. Key swap       -- a swap only ever inverts the relative order of that one pair,
    //      so exchanging just their two `key` values is the minimal edit that keeps
    //      "smaller key => higher rank" true for canRemoveMod's O(1) comparison everywhere
    //      else in the list, with zero renumbering.
    // A later re-add still lands at the tail correctly: `counter` only ever hands out
    // values larger than any key issued so far, and a key swap never manufactures a new
    // maximum -- it only exchanges two values that already existed.
    vector<string> solution(vector<string>& logs, vector<vector<string>>& queries) {
        unordered_map<string, Community> communities;

        // ---- replay the log to build current per-community state (same as Part 2) ----
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
            else if (method == "canRemoveMod") {
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
                            ok = ai->second->key < ti->second->key;   // maintained order, not raw ts
                    }
                }
                results.push_back(ok ? "true" : "false");
            }
            else { // "demote"
                const string& community = q[1];
                const string& user      = q[2];
                auto ci = communities.find(community);
                if (ci != communities.end()) {
                    Community& c = ci->second;
                    auto pit = c.pos.find(user);
                    if (pit != c.pos.end()) {
                        auto a = pit->second;
                        auto b = next(a);
                        if (b != c.order.end()) {              // not already lowest
                            swap(a->key, b->key);              // O(1): only this pair inverts order
                            c.order.splice(a, c.order, b);      // O(1) physical adjacent swap
                        }
                    }
                }
                results.push_back("null");                     // None -> keep 1 result per query
            }
        }

        return results;
    }
};
