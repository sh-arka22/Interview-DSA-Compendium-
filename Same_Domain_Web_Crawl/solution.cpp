#include <string>
#include <vector>
#include <map>
#include <queue>
#include <unordered_set>
using namespace std;

class Solution {
public:
    // host(url): the text between "://" and the next '/'. Tolerates a missing
    // scheme (start at 0) and a missing trailing slash (run to end of string).
    string host(const string& url) {
        size_t s = url.find("://");
        s = (s == string::npos) ? 0 : s + 3;
        size_t e = url.find('/', s);
        return url.substr(s, e == string::npos ? string::npos : e - s);
    }

    // root(host): the last two dot-separated labels. Two rfind calls from the
    // right — O(len), no vector split / allocation. "blog.example.com" ->
    // "example.com"; "example.com" -> "example.com".
    string root(const string& h) {
        size_t d1 = h.rfind('.');
        if (d1 == string::npos) return h;          // single label, no dot
        size_t d2 = h.rfind('.', d1 - 1);
        return (d2 == string::npos) ? h : h.substr(d2 + 1);
    }

    // BFS from seed. Two filters on top of a textbook queue-BFS:
    //   1. same-domain: keep a link only if its root domain == seed's root.
    //   2. dedupe: a URL enters the queue at most once.
    // The visited set is written on ENQUEUE (not on dequeue) so a URL linked
    // twice is still enqueued exactly once, and BFS order is preserved.
    vector<string> solution(string seed, map<string, vector<string>> graph) {
        string target = root(host(seed));
        vector<string> order;
        unordered_set<string> seen{seed};
        queue<string> q;
        q.push(seed);

        while (!q.empty()) {
            string u = q.front(); q.pop();
            order.push_back(u);                    // record in BFS visitation order

            auto it = graph.find(u);
            if (it == graph.end()) continue;       // link-only node = no out-edges

            for (const string& v : it->second) {   // links in listed order
                if (seen.count(v)) continue;       // dedupe / already queued
                if (root(host(v)) != target) continue; // drop cross-domain
                seen.insert(v);                    // mark on enqueue
                q.push(v);
            }
        }
        return order;
    }
};
