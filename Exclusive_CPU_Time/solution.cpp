#include <vector>
#include <string>
using namespace std;

class Solution {
    // Parse a log line "id:type:ts" -> function id, start-flag, timestamp.
    // Two ':' separators; type is "start" or "end", so its first char disambiguates.
    static void parseLog(const string& log, int& id, bool& isStart, int& ts) {
        size_t a = log.find(':');
        size_t b = log.find(':', a + 1);
        id      = stoi(log.substr(0, a));
        isStart = (log[a + 1] == 's');           // 's'tart vs 'e'nd
        ts      = stoi(log.substr(b + 1));
    }

public:
    vector<int> solution(int n, vector<string> logs) {
        vector<int> ans(n, 0);
        vector<int> stk;                         // ids of functions currently on the CPU
        int prev = 0;                            // time the current top last (re)started running

        for (const string& log : logs) {
            int id, ts; bool isStart;
            parseLog(log, id, isStart, ts);

            if (isStart) {
                // The running function (if any) held the CPU over [prev, ts-1].
                if (!stk.empty()) ans[stk.back()] += ts - prev;
                stk.push_back(id);
                prev = ts;
            } else {
                // end is inclusive: this function also owns timestamp ts.
                ans[stk.back()] += ts - prev + 1;
                stk.pop_back();
                prev = ts + 1;                   // caller resumes at the next unit
            }
        }
        return ans;
    }
};
