#include <vector>
#include <string>
#include <queue>
#include <utility>
using namespace std;

class Solution {
public:
    vector<string> solution(vector<vector<string>> ops) {
        // Min-heap ordered by expire: (expire, amount).
        // Every operation we care about -- "drop what's expired" and
        // "spend from whatever expires soonest" -- only ever touches the
        // MINIMUM-expiry grant. A heap gives O(log n) access to exactly
        // that element; we never need to reach into the middle of the
        // collection, so a full ordered structure (multiset/BST) would
        // pay higher constant-factor cost for a capability we don't use.
        priority_queue<pair<long long, long long>,
                       vector<pair<long long, long long>>,
                       greater<pair<long long, long long>>> heap;

        long long total = 0;   // sum of amounts currently sitting on the heap
        vector<string> result;
        result.reserve(ops.size());

        // Lazy deletion: grants aren't proactively swept when they expire
        // (there's no timer). We only discard expired grants when an
        // operation actually needs an accurate answer "as of" time t.
        // Since ops arrive in non-decreasing timestamp order, once a grant
        // expires relative to some t, it's expired for every later op too --
        // each grant is popped at most once, ever.
        auto dropExpired = [&](long long t) {
            while (!heap.empty() && heap.top().first <= t) {
                total -= heap.top().second;
                heap.pop();
            }
        };

        for (const auto& op : ops) {
            const string& type = op[0];
            long long ts = stoll(op[1]);

            if (type == "ADD_GRANT") {
                long long amount = stoll(op[2]);
                long long expire = stoll(op[3]);
                heap.push({expire, amount});
                total += amount;
            } 
            else if (type == "SPEND") {
                dropExpired(ts);
                long long remaining = stoll(op[2]);
                while (remaining > 0 && !heap.empty()) {
                    auto [expire, amount] = heap.top();
                    heap.pop();
                    if (amount <= remaining) {
                        // Fully consume this grant, move to the next-soonest.
                        remaining -= amount;
                        total -= amount;
                    } else {
                        // Partially consume it; the leftover keeps the same
                        // expiry, so it's pushed back and stays the new min.
                        total -= remaining;
                        heap.push({expire, amount - remaining});
                        remaining = 0;
                    }
                }
                // If remaining > 0 here, the balance ran out first --
                // per spec we just zero out and ignore the shortfall.
            } 
            else { // GET_BALANCE
                dropExpired(ts);
                result.push_back(to_string(total));
            }
        }
        return result;
    }
};
