#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <unordered_set>
using namespace std;

class Solution {
public:
    vector<vector<string>> solution(vector<vector<string>> operations) {
        vector<pair<string, string>> handlers;
        vector<string> logs;
        vector<unordered_set<string>> wordSets;  // pre-tokenized words per log
        vector<vector<string>> results;

        for (auto& op : operations) {
            if (op.empty()) continue;
            string& cmd = op[0];

            if (cmd == "add_handler") {
                handlers.push_back({op[1], op.size() >= 3 ? op[2] : ""});

            } else if (cmd == "add_log") {
                string r = op.size() >= 2 ? op[1] : "";
                for (auto& [type, text] : handlers) {
                    if (type == "upper"){
                        auto cmp = [&](unsigned char c){ return toupper(c); };
                        transform(r.begin(), r.end(), r.begin(),cmp);
                    }
                    else if (type == "prefix"){
                        r = text + r;
                    }
                    else if (type == "suffix"){
                        r = r + text;
                    }
                }
                logs.push_back(r);

                // Pre-tokenize once at insert time instead of on every search
                unordered_set<string> words;
                istringstream iss(r);
                string w;
                while (iss >> w) words.insert(w);
                wordSets.push_back(move(words));

            } else if (cmd == "get_logs") {
                vector<string> out;
                for (int i = 0; i < (int)logs.size(); ++i)
                    out.push_back(to_string(i + 1) + ":" + logs[i]);
                results.push_back(out);

            } else if (cmd == "search") {
                // LINEAR SCAN but O(1) per-log check via pre-built word set
                // Time: O(L) per search instead of O(L * W)
                string keyword = op.size() >= 2 ? op[1] : "";
                vector<string> out;
                if (!keyword.empty()) {
                    for (int i = 0; i < (int)logs.size(); ++i)
                        if (wordSets[i].count(keyword))
                            out.push_back(to_string(i + 1) + ":" + logs[i]);
                }
                results.push_back(out);
            }
        }
        return results;
    }
};
