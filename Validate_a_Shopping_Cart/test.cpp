// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <cassert>
#include <iostream>
using namespace std;

using Item = map<string, string>;
using Ret  = map<string, string>;

static void check(const string& name, bool cond) {
    if (!cond) { cerr << "FAIL  " << name << "\n"; exit(1); }
    cout << "PASS  " << name << "\n";
}

Solution sol;

// Helper: simulate platform's float-string serialisation of Python ints.
// E.g. qty=2 -> "2.0", qty=0 -> "0.0"
static string f(int n) { return to_string(n) + ".0"; }

// ── Example 1: valid cart (quantities as float strings "2.0", "1.0") ─────────
void test_valid() {
    auto r = sol.solution(
        {{{"itemId","burger"},{"quantity",f(2)}}, {{"itemId","fries"},{"quantity",f(1)}}},
        {{{"itemId","burger"},{"availableQuantity",f(5)},{"minQuantity",f(1)},{"maxQuantity",f(3)}},
         {{"itemId","fries"}, {"availableQuantity",f(10)},{"minQuantity",f(1)},{"maxQuantity",f(5)}}}
    );
    check("valid: isValid", r["isValid"] == "true");
    check("valid: errors",  r["errors"]  == "[]");
}

// ── Example 2: INVALID_QUANTITY + ITEM_NOT_FOUND + EXCEEDS_AVAILABLE ─────────
void test_example2() {
    auto r = sol.solution(
        {{{"itemId","burger"},{"quantity",f(4)}},
         {{"itemId","soda"},  {"quantity",f(0)}},    // qty 0 -> invalid
         {{"itemId","pizza"}, {"quantity",f(1)}}},   // not in catalog
        {{{"itemId","burger"},{"availableQuantity",f(3)},{"minQuantity",f(1)},{"maxQuantity",f(5)}},
         {{"itemId","soda"},  {"availableQuantity",f(10)},{"minQuantity",f(1)},{"maxQuantity",f(6)}}}
    );
    check("ex2: isValid",       r["isValid"] == "false");
    check("ex2: INVALID_QTY",   r["errors"].find("INVALID_QUANTITY")  != string::npos);
    check("ex2: NOT_FOUND",     r["errors"].find("ITEM_NOT_FOUND")    != string::npos);
    check("ex2: EXCEEDS_AVAIL", r["errors"].find("EXCEEDS_AVAILABLE") != string::npos);
    // Ordering: per-line errors before aggregate
    check("ex2: order",         r["errors"].find("INVALID_QUANTITY")  <
                                r["errors"].find("EXCEEDS_AVAILABLE"));
}

// ── String quantities (platform may also pass as plain "2") ───────────────────
void test_plain_string_quantities() {
    auto r = sol.solution(
        {{{"itemId","x"},{"quantity","3"}}},
        {{{"itemId","x"},{"availableQuantity","10"},{"minQuantity","1"},{"maxQuantity","5"}}}
    );
    check("plain_str: isValid", r["isValid"] == "true");
    check("plain_str: errors",  r["errors"]  == "[]");
}

// ── Empty cart ────────────────────────────────────────────────────────────────
void test_empty_cart() {
    auto r = sol.solution({}, {{{"itemId","x"},{"availableQuantity",f(5)},{"minQuantity",f(1)},{"maxQuantity",f(5)}}});
    check("empty: isValid",    r["isValid"] == "false");
    check("empty: EMPTY_CART", r["errors"].find("EMPTY_CART") != string::npos);
}

// ── Duplicate itemId: quantities aggregated ───────────────────────────────────
void test_duplicate_agg() {
    auto r = sol.solution(
        {{{"itemId","x"},{"quantity",f(2)}}, {{"itemId","x"},{"quantity",f(2)}}},
        {{{"itemId","x"},{"availableQuantity",f(3)},{"minQuantity",f(1)},{"maxQuantity",f(5)}}}
    );
    check("dup: isValid",       r["isValid"] == "false");
    check("dup: EXCEEDS_AVAIL", r["errors"].find("EXCEEDS_AVAILABLE") != string::npos);
}

// ── BELOW_MIN ──────────────────────────────────────────────────────────────
void test_below_min() {
    auto r = sol.solution(
        {{{"itemId","x"},{"quantity",f(1)}}},
        {{{"itemId","x"},{"availableQuantity",f(10)},{"minQuantity",f(3)},{"maxQuantity",f(8)}}}
    );
    check("below_min: code", r["errors"].find("BELOW_MIN_QUANTITY") != string::npos);
}

// ── EXCEEDS_MAX ───────────────────────────────────────────────────────────────
void test_exceeds_max() {
    auto r = sol.solution(
        {{{"itemId","x"},{"quantity",f(9)}}},
        {{{"itemId","x"},{"availableQuantity",f(20)},{"minQuantity",f(1)},{"maxQuantity",f(5)}}}
    );
    check("exceeds_max: code", r["errors"].find("EXCEEDS_MAX_QUANTITY") != string::npos);
}

// ── Boolean quantity string "True" is rejected ────────────────────────────────
void test_bool_qty() {
    auto r = sol.solution(
        {{{"itemId","x"},{"quantity","True"}}},
        {{{"itemId","x"},{"availableQuantity",f(10)},{"minQuantity",f(1)},{"maxQuantity",f(5)}}}
    );
    check("bool_qty: INVALID", r["errors"].find("INVALID_QUANTITY") != string::npos);
}

// ── isPosInt edge cases ───────────────────────────────────────────────────────
void test_isposint_edges() {
    // Re-use the solution's logic via small carts
    auto make = [&](const string& qty) -> map<string,string> {
        return sol.solution(
            {{{"itemId","x"},{"quantity",qty}}},
            {{{"itemId","x"},{"availableQuantity","100"},{"minQuantity","1"},{"maxQuantity","100"}}}
        );
    };
    check("isPosInt 0.0 invalid",  make("0.0")["isValid"] == "false");
    check("isPosInt 1.5 invalid",  make("1.5")["isValid"] == "false");
    check("isPosInt -1 invalid",   make("-1")["isValid"]  == "false");
    check("isPosInt 2.0 valid",    make("2.0")["isValid"] == "true");
    check("isPosInt 2.00 valid",   make("2.00")["isValid"]== "true");
    check("isPosInt 10.0 valid",   make("10.0")["isValid"]== "true");
}

int main() {
    test_valid();
    test_example2();
    test_plain_string_quantities();
    test_empty_cart();
    test_duplicate_agg();
    test_below_min();
    test_exceeds_max();
    test_bool_qty();
    test_isposint_edges();
    cout << "\nAll tests passed.\n";
}
