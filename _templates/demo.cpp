// g++ -std=c++17 -O2 -o demo demo.cpp && ./demo
#include "json_parser.hpp"
#include <iostream>
using namespace std;

static void check(const string& name, const string& got, const string& exp) {
    if (got != exp) {
        cerr << "FAIL  " << name << "\n  got: " << got << "\n  exp: " << exp << "\n";
        exit(1);
    }
    cout << "PASS  " << name << "\n";
}

int main() {
    // ── 1. Canonicalize a value: object keys get sorted, arrays keep order ──────
    {
        JsonParser jp(R"({"b":2,"a":1})");
        check("canonical object", jp.parseValue(), R"({"a":1,"b":2})");
    }
    {
        JsonParser jp(R"([3,1,2])");
        check("array order kept", jp.parseValue(), "[3,1,2]");
    }
    {
        JsonParser jp(R"({"z":{"d":4,"c":3},"a":[2,1]})");  // nested object sorted, array kept
        check("nested", jp.parseValue(), R"({"a":[2,1],"z":{"c":3,"d":4}})");
    }

    // ── 2. Extract fields from a record object ─────────────────────────────────
    {
        JsonParser jp(R"({"op":"put","key":"a","value":1})");
        auto rec = jp.parseObject();
        check("field op",    JsonParser::unquote(rec["\"op\""]),  "put");
        check("field key",   JsonParser::unquote(rec["\"key\""]), "a");
        check("field value", rec["\"value\""],                    "1");
        // Re-serialize -> canonical record (keys sorted: key < op < value)
        check("record canonical", JsonParser::serialize(rec),
              R"({"key":"a","op":"put","value":1})");
    }

    // ── 3. Walk the top-level {capacity, existing_log, operations} shape ────────
    {
        string input = R"({"capacity":2,"existing_log":[{"op":"put","key":"a","value":1}],)"
                       R"("operations":[{"op":"get","key":"a"},{"op":"put","key":"c","value":3}]})";
        JsonParser jp(input);
        auto root = jp.parseObject();

        check("capacity", root["\"capacity\""], "2");

        // Re-parse a nested-array string with a fresh parser
        JsonParser ops(root["\"operations\""]);
        auto records = ops.parseArray();
        check("num operations", to_string(records.size()), "2");

        // Inspect the first operation record
        JsonParser first(records[0]);
        auto r0 = first.parseObject();
        check("op0", JsonParser::unquote(r0["\"op\""]),  "get");
        check("key0", JsonParser::unquote(r0["\"key\""]), "a");
    }

    // ── 4. Whitespace / pretty-printed input is handled ────────────────────────
    {
        JsonParser jp("{ \"a\" : 1 , \"b\" : [ 2 , 3 ] }");
        check("pretty-printed", jp.parseValue(), R"({"a":1,"b":[2,3]})");
    }

    // ── 5. Escapes inside strings survive ──────────────────────────────────────
    {
        JsonParser jp(R"({"k":"a\"b"})");
        check("escaped quote", jp.parseValue(), R"({"k":"a\"b"})");
    }

    cout << "\nAll template checks passed.\n";
}
