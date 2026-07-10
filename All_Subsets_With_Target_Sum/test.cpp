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

// Reference: brute-force all 2^n index subsets of the sorted array, keep those
// summing to target, sort lexicographically. Validates content AND order.
static vector<vector<int>> brute(vector<int> nums, int target) {
    sort(nums.begin(), nums.end());
    int n = nums.size();
    vector<vector<int>> out;
    for (int mask = 0; mask < (1 << n); mask++) {
        long long s = 0;
        vector<int> sub;
        for (int i = 0; i < n; i++)
            if (mask >> i & 1) { s += nums[i]; sub.push_back(nums[i]); }
        if (s == target) out.push_back(sub);
    }
    sort(out.begin(), out.end());
    return out;
}

void test_example1() {
    auto r = sol.solution({2, 3, 7, 8, 10}, 11);
    check("ex1: [[3,8]]", r == vector<vector<int>>{{3, 8}});
}

void test_example2() {
    auto r = sol.solution({1, 2, 2}, 3);
    check("ex2: [[1,2],[1,2]] duplicates kept", r == vector<vector<int>>{{1, 2}, {1, 2}});
}

void test_no_solution() {
    check("no solution: empty", sol.solution({2, 4, 6}, 5).empty());
}

void test_target_zero() {
    auto r = sol.solution({1, 2}, 0);
    check("target 0: [[]]", r == vector<vector<int>>{{}});
}

void test_zeros() {
    auto r = sol.solution({0, 1}, 1);
    check("zeros: [[0,1],[1]]", r == brute({0, 1}, 1));
}

void test_single() {
    check("single hit", sol.solution({5}, 5) == vector<vector<int>>{{5}});
    check("single miss", sol.solution({4}, 5).empty());
}

void test_random_cross_check() {
    mt19937 rng(42);
    for (int iter = 0; iter < 300; iter++) {
        int n = rng() % 9;                       // 0..8 elements
        vector<int> nums(n);
        for (int& x : nums) x = rng() % 9;       // values 0..8 (with zeros & dups)
        int target = rng() % 20;                 // 0..19
        if (sol.solution(nums, target) != brute(nums, target)) {
            cerr << "FAIL  random cross-check iter " << iter << "\n";
            exit(1);
        }
    }
    cout << "PASS  300 random cases vs brute force (content + lexicographic order)\n";
}

void test_pruning_stress() {
    // 30 elements, mostly useless large values: feasibility DP must keep this instant.
    vector<int> nums;
    for (int i = 0; i < 28; i++) nums.push_back(1000 + i);
    nums.push_back(3);
    nums.push_back(8);
    auto r = sol.solution(nums, 11);
    check("stress: dead branches pruned", r == vector<vector<int>>{{3, 8}});
}

int main() {
    test_example1();
    test_example2();
    test_no_solution();
    test_target_zero();
    test_zeros();
    test_single();
    test_random_cross_check();
    test_pruning_stress();
    cout << "ALL TESTS PASSED\n";
    return 0;
}
