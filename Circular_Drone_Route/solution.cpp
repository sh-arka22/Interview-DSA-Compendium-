#include <vector>
using namespace std;

class Solution {
public:
    // corridor[i] = length between hub i and hub (i+1)%n.
    // route = ordered hub indices to visit.
    //
    // Key idea: a ring is just a line that wraps. Lay the corridors out as prefix
    // sums so that pre[i] = clockwise distance from hub 0 to hub i. Then between any
    // two hubs the *clockwise* arc is a plain difference of prefixes, and the
    // counterclockwise arc is whatever is left of the full loop. The leg cost is the
    // smaller of the two — no per-leg walking, each leg is O(1).
    long long solution(vector<int>& corridor, vector<int>& route) {
        int n = corridor.size();

        // pre has n+1 entries; pre[0] = 0, pre[n] = total loop length.
        vector<long long> pre(n + 1, 0);
        for (int i = 0; i < n; i++)
            pre[i + 1] = pre[i] + corridor[i];
        long long total = pre[n];

        long long res = 0;
        for (size_t j = 1; j < route.size(); j++) {
            int lo = route[j - 1], hi = route[j];
            if (lo > hi) { int t = lo; lo = hi; hi = t; }   // order the endpoints
            long long arc = pre[hi] - pre[lo];              // clockwise arc lo -> hi
            res += (arc < total - arc) ? arc : total - arc; // shorter of the two arcs
        }
        return res;
    }
};
