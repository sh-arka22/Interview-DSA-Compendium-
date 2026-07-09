// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <iostream>
using namespace std;

static void check(const string& name, vector<string> got, vector<string> exp) {
    if (got != exp) {
        cerr << "FAIL  " << name << "\n  got:";
        for (auto& s : got) cerr << " " << s;
        cerr << "\n  exp:";
        for (auto& s : exp) cerr << " " << s;
        cerr << "\n";
        exit(1);
    }
    cout << "PASS  " << name << " ->";
    for (auto& s : got) cout << " " << s;
    cout << "\n";
}

Solution sol;

int main() {
    // Example 1: other.com dropped (diff root); blog.example.com kept (shared
    // root); second sighting of /b and the back-link to seed not revisited.
    map<string, vector<string>> g1 = {
        {"https://example.com/", {"https://example.com/a", "https://blog.example.com/x", "https://other.com/"}},
        {"https://example.com/a", {"https://example.com/", "https://example.com/b"}},
        {"https://blog.example.com/x", {"https://other.com/", "https://example.com/b"}},
        {"https://example.com/b", {}}
    };
    check("ex1", sol.solution("https://example.com/", g1),
          {"https://example.com/", "https://example.com/a",
           "https://blog.example.com/x", "https://example.com/b"});

    // Example 2: seed is not a key -> no out-edges -> only the seed.
    check("ex2: seed absent", sol.solution("https://foo.com/", {}),
          {"https://foo.com/"});

    // Duplicate links on one page: /a listed twice -> enqueued once.
    map<string, vector<string>> g3 = {
        {"https://a.com/", {"https://a.com/x", "https://a.com/x", "https://a.com/y"}},
        {"https://a.com/x", {}}, {"https://a.com/y", {}}
    };
    check("dup links", sol.solution("https://a.com/", g3),
          {"https://a.com/", "https://a.com/x", "https://a.com/y"});

    // All neighbours cross-domain -> only the seed survives the filter.
    map<string, vector<string>> g4 = {
        {"https://a.com/", {"https://b.com/", "https://c.org/"}}
    };
    check("all cross-domain", sol.solution("https://a.com/", g4),
          {"https://a.com/"});

    // Subdomain seed: root of "shop.a.com" is "a.com"; a plain "a.com" link
    // shares that root and is kept.
    map<string, vector<string>> g5 = {
        {"https://shop.a.com/", {"https://a.com/p", "https://x.b.com/"}},
        {"https://a.com/p", {}}
    };
    check("subdomain seed", sol.solution("https://shop.a.com/", g5),
          {"https://shop.a.com/", "https://a.com/p"});

    // No trailing slash on host: "https://a.com" must still parse host "a.com".
    map<string, vector<string>> g6 = {
        {"https://a.com", {"https://a.com/deep"}},
        {"https://a.com/deep", {}}
    };
    check("no trailing slash", sol.solution("https://a.com", g6),
          {"https://a.com", "https://a.com/deep"});

    // BFS layering: level order respected across a branching graph.
    map<string, vector<string>> g7 = {
        {"https://s.com/", {"https://s.com/1", "https://s.com/2"}},
        {"https://s.com/1", {"https://s.com/3"}},
        {"https://s.com/2", {"https://s.com/4"}}
    };
    check("bfs order", sol.solution("https://s.com/", g7),
          {"https://s.com/", "https://s.com/1", "https://s.com/2",
           "https://s.com/3", "https://s.com/4"});

    cout << "\nAll tests passed.\n";
}
