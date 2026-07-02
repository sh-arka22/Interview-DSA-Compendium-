// ─────────────────────────────────────────────────────────────────────────────
// Minimal recursive-descent JSON parser for coding-platform problems.
//
// Designed for the common "parse a JSON string of records" interview format,
// e.g. {"op":"put","key":"a","value":1} or nested objects/arrays as keys.
//
// Three core operations, all O(n):
//   parseValue()  -> ANY value as a CANONICAL string  (object keys sorted)
//   parseObject() -> {"k":v,...}  as a map<string,string>  (field -> canonical value)
//   parseArray()  -> [a,b,...]    as a vector<string> of canonical elements
//
// Canonicalization: object keys are emitted alphabetically (via std::map), so
// {"b":2,"a":1} and {"a":1,"b":2} both serialize to {"a":1,"b":2}. Array order
// is always preserved. This makes values usable as deterministic hash keys.
//
// Tokens keep their JSON quotes: parseValue on "abc" returns the 5-char "\"abc\"".
// Use unquote() to strip them when you need the raw string.
//
// Handles: objects, arrays, strings (incl. \-escapes), numbers, true/false/null,
// and arbitrary surrounding whitespace (works on compact OR pretty-printed JSON).
//
// USAGE (inside a Solution): copy this struct in, then
//   JsonParser jp(inputJson);
//   auto root = jp.parseObject();           // top-level {...}
//   int cap   = stoi(root["\"capacity\""]);
//   JsonParser arr(root["\"operations\""]); // re-parse a nested array string
//   for (auto& rec : arr.parseArray()) { ... }
// ─────────────────────────────────────────────────────────────────────────────
#pragma once
#include <string>
#include <vector>
#include <map>
using namespace std;

struct JsonParser {
    string text;      // the JSON being parsed
    size_t pos = 0;   // current cursor

    explicit JsonParser(string s) : text(std::move(s)) {}

    void skipWs() {
        while (pos < text.size() && (unsigned char)text[pos] <= ' ') pos++;
    }

    // Parse ANY JSON value at the cursor; return its canonical string form.
    string parseValue() {
        skipWs();
        char c = text[pos];
        if (c == '{') { auto obj = parseObject(); return serialize(obj); }
        if (c == '[') {
            auto elems = parseArray();
            string out = "[";
            for (size_t k = 0; k < elems.size(); k++) {
                if (k) out += ',';
                out += elems[k];
            }
            return out + "]";
        }
        if (c == '"') {                              // string: copy verbatim incl. quotes
            size_t end = pos + 1;
            while (text[end] != '"') {
                if (text[end] == '\\') end++;        // skip the char after a backslash
                end++;
            }
            string token = text.substr(pos, end - pos + 1);
            pos = end + 1;
            return token;
        }
        size_t end = pos;                            // number / true / false / null
        while (end < text.size() && text[end] != ',' && text[end] != ']'
               && text[end] != '}' && (unsigned char)text[end] > ' ')
            end++;
        string token = text.substr(pos, end - pos);
        pos = end;
        return token;
    }

    // Parse {"k":v,...} at the cursor into a sorted {field -> canonical value} map.
    // Field keys retain their quotes, so look them up as obj["\"key\""].
    map<string, string> parseObject() {
        skipWs(); pos++;                             // skip '{'
        map<string, string> fields;
        skipWs();
        while (text[pos] != '}') {
            string key = parseValue();               // quoted string token
            skipWs(); pos++;                         // skip ':'
            fields[key] = parseValue();              // recursively canonical
            skipWs();
            if (text[pos] == ',') { pos++; skipWs(); }
        }
        pos++;                                       // skip '}'
        return fields;
    }

    // Parse [a,b,...] at the cursor into a vector of canonical element strings.
    vector<string> parseArray() {
        skipWs(); pos++;                             // skip '['
        vector<string> elems;
        skipWs();
        while (text[pos] != ']') {
            elems.push_back(parseValue());
            skipWs();
            if (text[pos] == ',') { pos++; skipWs(); }
        }
        pos++;                                       // skip ']'
        return elems;
    }

    // Rebuild a JSON object string from a map (keys already alphabetical in std::map).
    static string serialize(map<string, string>& fields) {
        string out = "{";
        bool first = true;
        for (auto& field : fields) {
            if (!first) out += ',';
            first = false;
            out += field.first + ':' + field.second; // field.first carries its quotes
        }
        return out + "}";
    }

    // "abc" -> abc ; leaves non-strings (numbers, objects) untouched.
    static string unquote(const string& s) {
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
            return s.substr(1, s.size() - 2);
        return s;
    }
};
