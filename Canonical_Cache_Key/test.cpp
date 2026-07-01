// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <cassert>
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

void test_example1() {
    check("ex1: no args/kwargs",
        sol.solution(R"({"func_name":"f","args":[],"kwargs":{}})"),
        R"(["f",[],{}])");
}

void test_example2() {
    check("ex2: kwargs reordered",
        sol.solution(R"({"func_name":"f","args":[],"kwargs":{"b":2,"a":1}})"),
        R"(["f",[],{"a":1,"b":2}])");
}

void test_args_preserved() {
    check("args order kept",
        sol.solution(R"({"func_name":"g","args":[3,1,2],"kwargs":{}})"),
        R"(["g",[3,1,2],{}])");
}

void test_nested_object_in_kwargs() {
    check("nested obj sorted",
        sol.solution(R"({"func_name":"h","args":[],"kwargs":{"z":{"d":4,"c":3}}})"),
        R"(["h",[],{"z":{"c":3,"d":4}}])");
}

void test_nested_object_in_args() {
    check("obj inside args sorted",
        sol.solution(R"({"func_name":"h","args":[{"y":2,"x":1}],"kwargs":{}})"),
        R"(["h",[{"x":1,"y":2}],{}])");
}

void test_mixed_value_types() {
    check("null/bool/num/str in args",
        sol.solution(R"({"func_name":"f","args":[null,true,false,42,"hi"],"kwargs":{}})"),
        R"(["f",[null,true,false,42,"hi"],{}])");
}

void test_fields_any_order() {
    check("top-level fields reordered",
        sol.solution(R"({"kwargs":{"b":1,"a":2},"args":[1,2],"func_name":"fn"})"),
        R"(["fn",[1,2],{"a":2,"b":1}])");
}

int main() {
    test_example1();
    test_example2();
    test_args_preserved();
    test_nested_object_in_kwargs();
    test_nested_object_in_args();
    test_mixed_value_types();
    test_fields_any_order();
    cout << "\nAll tests passed.\n";
}
