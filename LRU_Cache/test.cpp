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

// Example 1: eviction of key 2 then key 1
void test_example1() {
    auto r = sol.solution(2, {
        {1,1,1},{1,2,2},{0,1},{1,3,3},{0,2},{1,4,4},{0,1},{0,3},{0,4}
    });
    check("ex1", r == vector<int>({1, -1, -1, 3, 4}));
}

// Example 2: update makes key 1 MRU, so key 2 is evicted
void test_example2() {
    auto r = sol.solution(2, {
        {1,1,1},{1,2,2},{1,1,10},{1,3,3},{0,2},{0,1},{0,3}
    });
    check("ex2", r == vector<int>({-1, 10, 3}));
}

// get on empty cache
void test_get_missing() {
    auto r = sol.solution(1, {{0,1}});
    check("get_missing", r == vector<int>({-1}));
}

// capacity 1: each put evicts the previous
void test_capacity_one() {
    auto r = sol.solution(1, {
        {1,1,10},{0,1},{1,2,20},{0,1},{0,2}
    });
    check("cap1", r == vector<int>({10, -1, 20}));
}

// update (put on existing key) refreshes recency
void test_update_refreshes() {
    auto r = sol.solution(2, {
        {1,1,1},{1,2,2},{1,1,99},{1,3,3},   // update 1→99 makes 1 MRU; 3 evicts 2
        {0,1},{0,2},{0,3}
    });
    check("update_refresh", r == vector<int>({99, -1, 3}));
}

// all gets, no puts
void test_all_gets() {
    auto r = sol.solution(3, {{0,1},{0,2},{0,3}});
    check("all_gets", r == vector<int>({-1, -1, -1}));
}

int main() {
    test_example1();
    test_example2();
    test_get_missing();
    test_capacity_one();
    test_update_refreshes();
    test_all_gets();
    cout << "\nAll tests passed.\n";
}
