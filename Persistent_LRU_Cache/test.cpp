// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <iostream>
using namespace std;

static void check(const string& name, const string& got, const string& exp) {
    if (got != exp) {
        cerr << "FAIL  " << name << "\n  got: " << got << "\n  exp: " << exp << "\n";
        exit(1);
    }
    cout << "PASS  " << name << "\n";
}

Solution sol;

// Example 1: recover a,b; get(a)→1; put(c)→evicts b; get(b)→null
void test_example1() {
    string in = R"({"capacity":2,"existing_log":[{"op":"put","key":"a","value":1},{"op":"put","key":"b","value":2}],"operations":[{"op":"get","key":"a"},{"op":"put","key":"c","value":3},{"op":"get","key":"b"}]})";
    string exp = R"({"final_cache":[["a",1],["c",3]],"get_results":[1,null],"updated_log":[{"key":"a","op":"put","value":1},{"key":"b","op":"put","value":2},{"hit":true,"key":"a","op":"get"},{"key":"c","op":"put","value":3},{"hit":false,"key":"b","op":"get"}]})";
    check("example1", sol.solution(in), exp);
}

// Example 2: malformed tail ignored; get(x)→1; put(z,3) no eviction
void test_example2() {
    string in = R"({"capacity":2,"existing_log":[{"op":"put","key":"x","value":1},"MALFORMED"],"operations":[{"op":"get","key":"x"},{"op":"put","key":"z","value":3}]})";
    string exp = R"({"final_cache":[["x",1],["z",3]],"get_results":[1],"updated_log":[{"key":"x","op":"put","value":1},{"hit":true,"key":"x","op":"get"},{"key":"z","op":"put","value":3}]})";
    check("example2_malformed", sol.solution(in), exp);
}

// Empty log and no operations
void test_empty() {
    string in = R"({"capacity":3,"existing_log":[],"operations":[]})";
    string exp = R"({"final_cache":[],"get_results":[],"updated_log":[]})";
    check("empty", sol.solution(in), exp);
}

// Log has a get record (LRU order matters for replay)
void test_recovery_with_get() {
    // put(a,1), put(b,2), get(a) → a is now MRU, b is LRU
    // then put(c,3) from operations → evicts b (LRU), not a
    string in = R"({"capacity":2,"existing_log":[{"op":"put","key":"a","value":1},{"op":"put","key":"b","value":2},{"hit":true,"key":"a","op":"get"}],"operations":[{"op":"put","key":"c","value":3},{"op":"get","key":"b"},{"op":"get","key":"a"}]})";
    string exp = R"({"final_cache":[["a",1],["c",3]],"get_results":[null,1],"updated_log":[{"key":"a","op":"put","value":1},{"key":"b","op":"put","value":2},{"hit":true,"key":"a","op":"get"},{"key":"c","op":"put","value":3},{"hit":false,"key":"b","op":"get"},{"hit":true,"key":"a","op":"get"}]})";
    check("recovery_with_get", sol.solution(in), exp);
}

// kwargs-style key: object key needs canonical sorting
void test_object_key() {
    // key is {"b":2,"a":1} which canonicalizes to {"a":1,"b":2}
    string in = R"({"capacity":2,"existing_log":[],"operations":[{"op":"put","key":{"b":2,"a":1},"value":99},{"op":"get","key":{"a":1,"b":2}}]})";
    string exp = R"({"final_cache":[[{"a":1,"b":2},99]],"get_results":[99],"updated_log":[{"key":{"a":1,"b":2},"op":"put","value":99},{"hit":true,"key":{"a":1,"b":2},"op":"get"}]})";
    check("object_key", sol.solution(in), exp);
}

int main() {
    test_empty();
    test_example1();
    test_example2();
    test_recovery_with_get();
    test_object_key();
    cout << "\nAll tests passed.\n";
}
