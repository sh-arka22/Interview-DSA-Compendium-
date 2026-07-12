// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <iostream>
using namespace std;

Solution sol;

static string join(const vector<string>& v) {
    string s = "[";
    for (size_t i = 0; i < v.size(); i++) s += (i ? ", " : "") + v[i];
    return s + "]";
}

static void check(const string& name, vector<string> in, vector<string> exp) {
    auto got = sol.solution(in);
    if (got != exp) {
        cerr << "FAIL  " << name << "\n  got: " << join(got) << "\n  exp: " << join(exp) << "\n";
        exit(1);
    }
    cout << "PASS  " << name << " -> " << join(got) << "\n";
}

int main() {
    // Example 1: "d1g" is not shorter than "dog" -> keep the word.
    check("ex1", {"dog", "deer", "deal"}, {"dog", "d2r", "d2l"});

    // Example 2: collide at k=1 ("a5z") and k=2 ("ab4z"); unique at k=3.
    check("ex2", {"abcdefz", "abxdefz"}, {"abc3z", "abx3z"});

    // Classic LC527 case: mixed groups, several conflict depths.
    check("lc527",
          {"like", "god", "internal", "me", "internet", "interval", "intension", "face", "intrusion"},
          {"l2e", "god", "internal", "me", "i6t", "interval", "inte4n", "f2e", "intr4n"});

    // Single word: k=1 wins when it saves length.
    check("single", {"apple"}, {"a3e"});

    // Length-2 and length-3 words can never be abbreviated shorter.
    check("too short", {"ab", "abc"}, {"ab", "abc"});

    // Same length, same last char, LCP = len-2 (max possible for distinct words):
    // k = len-1, abbreviation not shorter -> both originals kept.
    check("deep conflict keeps originals", {"aaaaab", "aaaacb"}, {"aaaaab", "aaaacb"});

    // Same word-shape but different last chars -> no conflict at all.
    check("last char splits groups", {"abcdx", "abcdy"}, {"a3x", "a3y"});

    // Same last char but different lengths -> no conflict.
    check("length splits groups", {"abcz", "abcdz"}, {"a2z", "a3z"});

    // Three-way conflict, uneven depths: internals/internets share LCP 6 (k=7,
    // abbreviation "interna1s"/"interne1s" not shorter -> originals kept), while
    // intervals only shares LCP 5 -> k=6 -> "interv2s".
    check("adjacent max propagation",
          {"internets", "internals", "intervals"},
          {"internets", "internals", "interv2s"});

    cout << "\nAll tests passed.\n";
    return 0;
}
