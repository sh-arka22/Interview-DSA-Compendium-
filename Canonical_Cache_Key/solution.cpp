#include <string>
#include <map>
using namespace std;

class Solution {
    string inp;
    int p;

    void ws() {
        while (p < (int)inp.size() && (unsigned char)inp[p] <= ' ') p++;
    }

    // Parse a JSON string, return it with surrounding double-quotes preserved
    string pStr() {
        string r = "\"";
        p++;  // skip opening "
        while (p < (int)inp.size()) {
            char c = inp[p++];
            if (c == '\\') { r += c; r += inp[p++]; }  // preserve escape
            else if (c == '"') { r += '"'; break; }
            else r += c;
        }
        return r;
    }

    // Parse a JSON array; preserve element order
    string pArr() {
        p++;  // skip '[
        string r = "[";
        ws();
        while (p < (int)inp.size() && inp[p] != ']') {
            if (r != "[") { if (inp[p] == ',') p++; r += ','; ws(); }
            r += pVal();
            ws();
        }
        p++;  // skip ']'
        return r + "]";
    }

    // Parse a JSON object; sort keys alphabetically (recursive via pVal)
    string pObj() {
        p++;  // skip '{'
        map<string, string> m;  // std::map keeps keys sorted
        ws();
        while (p < (int)inp.size() && inp[p] != '}') {
            ws();
            string key = pStr(); key = key.substr(1, key.size() - 2);  // strip quotes
            ws(); p++;           // skip ':'
            m[key] = pVal();
            ws();
            if (p < (int)inp.size() && inp[p] == ',') p++;
        }
        p++;  // skip '}'
        string r = "{";
        bool first = true;
        for (auto& kv : m) {
            if (!first) r += ',';
            first = false;
            r += '"' + kv.first + "\":" + kv.second;
        }
        return r + "}";
    }

    // Dispatch to the right parser by looking at the next character
    string pVal() {
        ws();
        char c = inp[p];
        if (c == '"') return pStr();
        if (c == '[') return pArr();
        if (c == '{') return pObj();
        if (c == 'n') { p += 4; return "null"; }
        if (c == 't') { p += 4; return "true"; }
        if (c == 'f') { p += 5; return "false"; }
        // number: read until delimiter
        string r;
        while (p < (int)inp.size() && inp[p] != ',' && inp[p] != ']'
               && inp[p] != '}' && (unsigned char)inp[p] > ' ')
            r += inp[p++];
        return r;
    }

public:
    string solution(string inputJson) {
        inp = std::move(inputJson);
        p = 0;
        ws(); p++;  // skip top-level '{'

        string fn, args = "[]", kwargs = "{}";
        while (p < (int)inp.size() && inp[p] != '}') {
            ws();
            string key = pStr(); key = key.substr(1, key.size() - 2);
            ws(); p++;  // skip ':'
            if      (key == "func_name") fn     = pVal();
            else if (key == "args")      args   = pVal();
            else if (key == "kwargs")    kwargs = pVal();
            else                         pVal();  // skip unknown fields
            ws();
            if (p < (int)inp.size() && inp[p] == ',') p++;
        }

        return "[" + fn + "," + args + "," + kwargs + "]";
    }
};
