#include <string>
#include <list>
#include <unordered_map>
#include <vector>
using namespace std;

class Solution {
    // ── JSON parser state ─────────────────────────────────────────────────
    string txt; int p;
    void ws() { while (p < (int)txt.size() && (unsigned char)txt[p] <= ' ') p++; }

    string pStr() {                                  // "hello" → hello
        p++; string r;
        while (txt[p] != '"') {
            if (txt[p] == '\\') { p++; r += txt[p++]; }
            else r += txt[p++];
        }
        p++; return r;
    }

    // Parse a JSON array of strings: ["a","b"] → vector<string>
    vector<string> pStrArr() {
        vector<string> v; ws(); p++; ws();           // skip '['
        while (txt[p] != ']') {
            ws(); v.push_back(pStr());
            ws(); if (txt[p] == ',') p++;
        }
        p++; return v;                               // skip ']'
    }

    // Parse a JSON array of arrays: [["a"],["b","c"]] → vector<vector<string>>
    vector<vector<string>> pArrArr() {
        vector<vector<string>> v; ws(); p++; ws();   // skip outer '['
        while (txt[p] != ']') {
            ws(); v.push_back(pStrArr());
            ws(); if (txt[p] == ',') p++;
        }
        p++; return v;
    }

    // ── CSV splitter (logs come as "target,action,actor,timestamp") ──────
    static vector<string> splitCSV(const string& s) {
        vector<string> out; string cur;
        for (char c : s) {
            if (c == ',') { out.push_back(cur); cur.clear(); }
            else cur += c;
        }
        out.push_back(cur); return out;
    }

    struct Entry { string user; long long key; };

public:
    string __solve(const string& input) {
        txt = input; p = 0;

        // ── Parse top-level JSON: {"logs":[...], "queries":[...]} ─────────
        vector<string> logs;
        vector<vector<string>> queries;
        ws(); p++; ws();                             // skip '{'
        while (txt[p] != '}') {
            ws(); string key = pStr(); ws(); p++;    // key + ':'
            if      (key == "logs")    logs    = pStrArr();
            else if (key == "queries") queries = pArrArr();
            ws(); if (txt[p] == ',') p++;
        }

        // ── Core logic: linked-list ranked moderator set ─────────────────
        list<Entry> order;                           // head=highest .. tail=lowest rank
        unordered_map<string, list<Entry>::iterator> pos;
        long long counter = 0;

        for (auto& log : logs) {
            auto f = splitCSV(log);
            const string& target = f[0];
            const string& action = f[1];
            if (action == "added") {
                order.push_back({target, counter++});
                pos[target] = prev(order.end());
            } else {
                order.erase(pos[target]);
                pos.erase(target);
            }
        }

        // ── Answer queries ───────────────────────────────────────────────
        string out = "[";
        bool first = true;
        for (auto& q : queries) {
            if (!first) out += ',';
            first = false;

            if (q[0] == "getModRanking") {
                out += "[";
                bool f2 = true;
                for (auto& e : order) {
                    if (!f2) out += ',';
                    f2 = false;
                    out += "\"" + e.user + "\"";
                }
                out += "]";
            }
            else {                                   // "canRemoveMod"
                const string& target = q[1];
                const string& actor  = q[2];
                bool ok = false;
                if (target != actor) {
                    auto ti = pos.find(target), ai = pos.find(actor);
                    if (ti != pos.end() && ai != pos.end())
                        ok = ai->second->key < ti->second->key;
                }
                out += ok ? "true" : "false";
            }
        }
        return out + "]";
    }
};
