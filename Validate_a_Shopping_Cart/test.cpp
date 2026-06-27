// g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <cassert>
#include <iostream>
using namespace std;

using Item   = map<string, string>;
using Items  = vector<Item>;
using Ret    = map<string, string>;

static void check(const string& name, bool cond) {
    if (!cond) { cerr << "FAIL  " << name << "\n"; exit(1); }
    cout << "PASS  " << name << "\n";
}

Solution sol;

// ── Example 1: valid cart ─────────────────────────────────────────────────────
void test_valid() {
    Items cart    = {{{"itemId","burger"},{"quantity","2"}}, {{"itemId","fries"},{"quantity","1"}}};
    Items catalog = {{{"itemId","burger"},{"availableQuantity","5"},{"minQuantity","1"},{"maxQuantity","3"}},
                     {{"itemId","fries"}, {"availableQuantity","10"},{"minQuantity","1"},{"maxQuantity","5"}}};
    Ret r = sol.solution(cart, catalog);
    check("valid: isValid", r["isValid"] == "true");
    check("valid: errors",  r["errors"]  == "[]");
}

// ── Example 2: INVALID_QUANTITY + ITEM_NOT_FOUND + EXCEEDS_AVAILABLE ─────────
void test_example2() {
    Items cart    = {{{"itemId","burger"},{"quantity","4"}},
                     {{"itemId","soda"},  {"quantity","0"}},
                     {{"itemId","pizza"}, {"quantity","1"}}};
    Items catalog = {{{"itemId","burger"},{"availableQuantity","3"},{"minQuantity","1"},{"maxQuantity","5"}},
                     {{"itemId","soda"},  {"availableQuantity","10"},{"minQuantity","1"},{"maxQuantity","6"}}};
    Ret r = sol.solution(cart, catalog);
    check("ex2: isValid",       r["isValid"] == "false");
    check("ex2: INVALID_QTY",   r["errors"].find("INVALID_QUANTITY") != string::npos);
    check("ex2: NOT_FOUND",     r["errors"].find("ITEM_NOT_FOUND")   != string::npos);
    check("ex2: EXCEEDS_AVAIL", r["errors"].find("EXCEEDS_AVAILABLE")!= string::npos);
    // Per-line errors must come before aggregate
    check("ex2: order",         r["errors"].find("INVALID_QUANTITY") <
                                r["errors"].find("EXCEEDS_AVAILABLE"));
}

// ── Empty cart ────────────────────────────────────────────────────────────────
void test_empty_cart() {
    Ret r = sol.solution({}, {{{"itemId","x"},{"availableQuantity","5"},{"minQuantity","1"},{"maxQuantity","5"}}});
    check("empty: isValid",    r["isValid"] == "false");
    check("empty: EMPTY_CART", r["errors"].find("EMPTY_CART") != string::npos);
}

// ── Duplicate itemId: quantities aggregated → EXCEEDS_AVAILABLE ───────────────
void test_duplicate_aggregation() {
    Items cart    = {{{"itemId","burger"},{"quantity","2"}}, {{"itemId","burger"},{"quantity","2"}}};
    Items catalog = {{{"itemId","burger"},{"availableQuantity","3"},{"minQuantity","1"},{"maxQuantity","5"}}};
    Ret r = sol.solution(cart, catalog);
    check("dup: isValid",       r["isValid"] == "false");
    check("dup: EXCEEDS_AVAIL", r["errors"].find("EXCEEDS_AVAILABLE") != string::npos);
}

// ── BELOW_MIN_QUANTITY ────────────────────────────────────────────────────────
void test_below_min() {
    Items cart    = {{{"itemId","x"},{"quantity","1"}}};
    Items catalog = {{{"itemId","x"},{"availableQuantity","10"},{"minQuantity","3"},{"maxQuantity","8"}}};
    Ret r = sol.solution(cart, catalog);
    check("below_min: false", r["isValid"] == "false");
    check("below_min: code",  r["errors"].find("BELOW_MIN_QUANTITY") != string::npos);
}

// ── EXCEEDS_MAX_QUANTITY ──────────────────────────────────────────────────────
void test_exceeds_max() {
    Items cart    = {{{"itemId","x"},{"quantity","9"}}};
    Items catalog = {{{"itemId","x"},{"availableQuantity","20"},{"minQuantity","1"},{"maxQuantity","5"}}};
    Ret r = sol.solution(cart, catalog);
    check("exceeds_max: false", r["isValid"] == "false");
    check("exceeds_max: code",  r["errors"].find("EXCEEDS_MAX_QUANTITY") != string::npos);
}

// ── Boolean quantity "True" is not a valid integer ───────────────────────────
void test_boolean_quantity() {
    Items cart    = {{{"itemId","x"},{"quantity","True"}}};
    Items catalog = {{{"itemId","x"},{"availableQuantity","10"},{"minQuantity","1"},{"maxQuantity","5"}}};
    Ret r = sol.solution(cart, catalog);
    check("bool_qty: false",    r["isValid"] == "false");
    check("bool_qty: INVALID",  r["errors"].find("INVALID_QUANTITY") != string::npos);
}

// ── Error ordering: index-0 per-line, index-2 per-line, then aggregate ────────
void test_error_ordering() {
    Items cart    = {{{"itemId","apple"}, {"quantity","0"}},
                     {{"itemId","banana"},{"quantity","5"}},
                     {{"itemId","cherry"},{"quantity","1"}}};
    Items catalog = {{{"itemId","apple"}, {"availableQuantity","5"}, {"minQuantity","1"},{"maxQuantity","5"}},
                     {{"itemId","banana"},{"availableQuantity","10"},{"minQuantity","10"},{"maxQuantity","20"}}};
    Ret r = sol.solution(cart, catalog);
    const string& e = r["errors"];
    check("order: all three",   e.find("INVALID_QUANTITY") != string::npos &&
                                e.find("ITEM_NOT_FOUND")   != string::npos &&
                                e.find("BELOW_MIN")        != string::npos);
    check("order: line before agg", e.find("INVALID_QUANTITY") < e.find("BELOW_MIN"));
}

int main() {
    test_valid();
    test_example2();
    test_empty_cart();
    test_duplicate_aggregation();
    test_below_min();
    test_exceeds_max();
    test_boolean_quantity();
    test_error_ordering();
    cout << "\nAll tests passed.\n";
}
