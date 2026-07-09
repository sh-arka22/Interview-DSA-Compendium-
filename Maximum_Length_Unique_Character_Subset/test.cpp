// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <cassert>
#include <iostream>
#include <random>
using namespace std;

static void check(const string& name, bool cond) {
    if (!cond) { cerr << "FAIL  " << name << "\n"; exit(1); }
    cout << "PASS  " << name << "\n";
}

Solution sol;

void test_example1() {
    vector<string> w = {"un", "iq", "ue"};
    check("ex1: un+iq=4", sol.solution(w) == 4);
}

void test_example2() {
    vector<string> w = {"cha", "r", "act", "ers"};
    check("ex2: act+ers=6", sol.solution(w) == 6);
}

void test_example3_all_internal_dup() {
    vector<string> w = {"aa", "bb"};
    check("ex3: all internal dup=0", sol.solution(w) == 0);
}

void test_single_word() {
    vector<string> w = {"abc"};
    check("single word=3", sol.solution(w) == 3);
}

void test_single_word_with_dup() {
    vector<string> w = {"aab"};
    check("single word w/ dup=0", sol.solution(w) == 0);
}

void test_all_disjoint() {
    vector<string> w = {"abc", "def", "ghi", "jkl"};
    check("all disjoint=12", sol.solution(w) == 12);
}

void test_overlap_pick_best_one() {
    vector<string> w = {"abc", "bcd"};
    check("overlap: best single=3", sol.solution(w) == 3);
}

void test_full_alphabet_single_word() {
    vector<string> w = {"abcdefghijklmnopqrstuvwxyz"};
    check("full alphabet=26", sol.solution(w) == 26);
}

void test_mix_usable_unusable() {
    vector<string> w = {"aa", "b", "cd"};
    check("mix usable/unusable=3", sol.solution(w) == 3);
}

void test_sixteen_words_worst_case() {
    // 16 disjoint single letters -> all combinable, length 16
    vector<string> w;
    for (char c = 'a'; c < 'a' + 16; c++) w.push_back(string(1, c));
    check("16 disjoint singles=16", sol.solution(w) == 16);
}

void test_early_exit_at_26() {
    // Two disjoint 13-letter halves of the alphabet -> hits the full 26 cap,
    // plus extra junk words that must never be explored once best == 26.
    vector<string> w = {"abcdefghijklm", "nopqrstuvwxyz", "aa", "zz", "ab"};
    check("early exit at 26", sol.solution(w) == 26);
}

// Brute-force reference: plain 2^n subset enumeration (no pruning), used to
// cross-check the optimized backtracking DFS on random inputs.
static int bruteForce(const vector<string>& words) {
    vector<int> masks;
    for (auto& w : words) {
        int m = 0; bool ok = true;
        for (char c : w) {
            int bit = 1 << (c - 'a');
            if (m & bit) { ok = false; break; }
            m |= bit;
        }
        if (ok) masks.push_back(m);
    }
    int n = masks.size(), best = 0;
    for (int s = 0; s < (1 << n); s++) {
        int combined = 0, len = 0;
        for (int i = 0; i < n; i++) if (s >> i & 1) {
            combined |= masks[i];
            len += __builtin_popcount(masks[i]);
        }
        if (__builtin_popcount(combined) == len) best = max(best, len);
    }
    return best;
}

void test_random_cross_check() {
    mt19937 rng(42);
    uniform_int_distribution<int> lenDist(1, 6);
    uniform_int_distribution<int> charDist(0, 4); // small alphabet -> frequent overlaps
    for (int trial = 0; trial < 300; trial++) {
        int n = 1 + trial % 12;
        vector<string> w;
        for (int i = 0; i < n; i++) {
            int L = lenDist(rng);
            string s;
            for (int j = 0; j < L; j++) s += char('a' + charDist(rng));
            w.push_back(s);
        }
        int expected = bruteForce(w);
        int got = sol.solution(w);
        if (got != expected) {
            cerr << "FAIL  random trial " << trial << ": expected " << expected
                 << " got " << got << " words=[";
            for (auto& s : w) cerr << s << ",";
            cerr << "]\n";
            exit(1);
        }
    }
    cout << "PASS  random cross-check vs brute force (300 trials)\n";
}

int main() {
    test_example1();
    test_example2();
    test_example3_all_internal_dup();
    test_single_word();
    test_single_word_with_dup();
    test_all_disjoint();
    test_overlap_pick_best_one();
    test_full_alphabet_single_word();
    test_mix_usable_unusable();
    test_sixteen_words_worst_case();
    test_early_exit_at_26();
    test_random_cross_check();
    cout << "\nAll tests passed.\n";
}
