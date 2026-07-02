#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

class Solution {
public:
    // Design: ONE live map that always holds the current visible state, plus a
    // per-transaction UNDO LOG so ROLLBACK can reverse only the innermost txn.
    // Reads (GET/COUNT) are O(1) because writes are applied immediately to `live`
    // and `valueCount` is kept in sync incrementally — no stack-of-maps scan.
    vector<string> solution(vector<vector<string>>& commands) {
        vector<string> out;
        unordered_map<string,string> live;       // the ONE true current state
        unordered_map<string,int> valueCount;    // value -> #keys holding it, maintained incrementally

        struct UndoRec { string key; bool hadValue; string prevValue; };
        vector<vector<UndoRec>> frames;          // one undo-log frame per open BEGIN

        auto applyChange = [&](const string& key, bool newHasValue, const string& newValue){
            auto it = live.find(key);
            if (it != live.end()) {
                auto cit = valueCount.find(it->second);
                if (--(cit->second) == 0) valueCount.erase(cit);
            }
            if (newHasValue) {
                live[key] = newValue;
                valueCount[newValue]++;
            } else if (it != live.end()) {
                live.erase(it);
            }
        };

        auto recordUndo = [&](const string& key){
            if (frames.empty()) return;              // no open txn -> write is already permanent
            auto it = live.find(key);
            if (it != live.end()) frames.back().push_back({key, true, it->second});
            else                   frames.back().push_back({key, false, ""});
        };

        for (auto& cmd : commands) {
            const string& op = cmd[0];
            if (op == "SET") {
                recordUndo(cmd[1]);
                applyChange(cmd[1], true, cmd[2]);
            } else if (op == "GET") {
                auto it = live.find(cmd[1]);
                out.push_back(it != live.end() ? it->second : "NULL");
            } else if (op == "DELETE") {
                if (live.count(cmd[1])) {             // missing key: genuine no-op, nothing to journal
                    recordUndo(cmd[1]);
                    applyChange(cmd[1], false, "");
                }
            } else if (op == "COUNT") {
                auto it = valueCount.find(cmd[1]);
                out.push_back(to_string(it != valueCount.end() ? it->second : 0));
            } else if (op == "BEGIN") {
                frames.push_back({});
            } else if (op == "COMMIT") {
                if (frames.empty()) out.push_back("NO TRANSACTION");
                else frames.clear();                  // flatten ALL open levels at once — O(records), not O(N)
            } else if (op == "ROLLBACK") {
                if (frames.empty()) out.push_back("NO TRANSACTION");
                else {
                    auto& frame = frames.back();
                    for (auto it = frame.rbegin(); it != frame.rend(); ++it)
                        applyChange(it->key, it->hadValue, it->prevValue);
                    frames.pop_back();                // only innermost closes
                }
            }
        }
        return out;
    }
};
