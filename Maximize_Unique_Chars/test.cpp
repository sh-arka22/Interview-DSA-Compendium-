// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <cassert>
#include <iostream>
using namespace std;

static void check(const string& name, bool cond) {
    if (!cond) { cerr << "FAIL  " << name << "\n"; exit(1); }
    cout << "PASS  " << name << "\n";
}

Solution sol;

void test_example1() {
    vector<string> w = {"ab","cd","aef","gh"};
    check("ex1: cd+aef+gh=7", sol.solution(w) == 7);
}

void test_example2() {
    vector<string> w = {};
    check("ex2: empty=0", sol.solution(w) == 0);
}

void test_word_with_internal_dup() {
    vector<string> w = {"aa","bb"};   // both unusable
    check("internal_dup: 0", sol.solution(w) == 0);
}

void test_all_same_chars() {
    vector<string> w = {"abc","bcd"};  // overlap; best is "abc" or "bcd" alone (len 3)
    check("overlap: 3", sol.solution(w) == 3);
}

void test_all_disjoint() {
    vector<string> w = {"abc","def","ghi","jkl"};
    check("disjoint: 12", sol.solution(w) == 12);
}

void test_single_word_26_chars() {
    vector<string> w = {"abcdefghijklmnopqrstuvwxyz"};
    check("all26: 26", sol.solution(w) == 26);
}

void test_mix_usable_unusable() {
    // "aa" unusable; "b" + "cd" = 3
    vector<string> w = {"aa","b","cd"};
    check("mix: 3", sol.solution(w) == 3);
}

int main() {
    test_example1();
    test_example2();
    test_word_with_internal_dup();
    test_all_same_chars();
    test_all_disjoint();
    test_single_word_26_chars();
    test_mix_usable_unusable();
    cout << "\nAll tests passed.\n";
}
