#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
using namespace std;

// ┌─────────────────────────────────────────────────────────────────────────────┐
// │  WHAT CHANGED FROM PART 1 → PART 2:                                       │
// │                                                                            │
// │  1. ADDED: unordered_map<string, vector<int>> index                        │
// │     — inverted index mapping each word to a list of log ids containing it  │
// │                                                                            │
// │  2. ADDED: index-building logic inside "add_log"                           │
// │     — after storing the log, tokenize it by whitespace                     │
// │     — for each UNIQUE word, append this log's id to index[word]            │
// │     — uses unordered_set<string> seen to deduplicate words within one log  │
// │     — this costs an extra O(W) per add_log (W = unique words)             │
// │                                                                            │
// │  3. CHANGED: "search" from linear scan → index lookup                     │
// │     BEFORE (Part 1): scan ALL logs, tokenize each, check word match       │
// │       → O(L * W) per search                                               │
// │     AFTER  (Part 2): index.find(keyword) → get matching log ids directly  │
// │       → O(K) per search (K = matching logs only)                          │
// │                                                                            │
// │  TRADE-OFF: add_log is slightly heavier (builds index), but search is     │
// │  dramatically faster when L is large and K is small.                       │
// └─────────────────────────────────────────────────────────────────────────────┘

class Solution {
public:
    vector<vector<string>> solution(vector<vector<string>> operations) {
        vector<pair<string, string>> handlers;
        vector<string> logs;
        unordered_map<string, vector<int>> index;   // ← NEW: word → [log ids]
        vector<vector<string>> results;

        for (auto& op : operations) {
            if (op.empty()) continue;
            string& cmd = op[0];

            if (cmd == "add_handler") {
                handlers.push_back({op[1], op.size() >= 3 ? op[2] : ""});

            } else if (cmd == "add_log") {
                // Apply handler pipeline in order (same as Part 1)
                string r = op.size() >= 2 ? op[1] : "";
                for (auto& [type, text] : handlers) {
                    if (type == "upper")
                        transform(r.begin(), r.end(), r.begin(),
                                  [](unsigned char c){ return toupper(c); });
                    else if (type == "prefix")
                        r = text + r;
                    else if (type == "suffix")
                        r = r + text;
                }

                int id = (int)logs.size() + 1;
                logs.push_back(r);

                // ── NEW: build inverted index ──
                // Tokenize transformed message, add this log's id once per unique word.
                // 'seen' set prevents duplicate ids when a word appears multiple times
                // e.g. "HELLO HELLO WORLD" → index["HELLO"] gets id only once.
                unordered_set<string> seen;
                istringstream iss(r);
                string word;
                while (iss >> word)
                    if (seen.insert(word).second)
                        index[word].push_back(id);

            } else if (cmd == "get_logs") {
                // Same as Part 1
                vector<string> out;
                for (int i = 0; i < (int)logs.size(); ++i)
                    out.push_back(to_string(i + 1) + ":" + logs[i]);
                results.push_back(out);

            } else if (cmd == "search") {
                // ── CHANGED: O(K) index lookup instead of O(L*W) linear scan ──
                string keyword = op.size() >= 2 ? op[1] : "";
                vector<string> out;
                if (!keyword.empty()) {
                    auto it = index.find(keyword);
                    if (it != index.end())
                        for (int id : it->second)
                            out.push_back(to_string(id) + ":" + logs[id - 1]);
                }
                results.push_back(out);
            }
        }
        return results;
    }
};
