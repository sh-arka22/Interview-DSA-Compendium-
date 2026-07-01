#include <string>
#include <map>
#include <list>
#include <unordered_map>
using namespace std;

/*
 WORKED EXAMPLE (Example 1)
 input = {"capacity":2,
          "existing_log":[{"op":"put","key":"a","value":1},
                          {"op":"put","key":"b","value":2}],
          "operations":  [{"op":"get","key":"a"},
                          {"op":"put","key":"c","value":3},
                          {"op":"get","key":"b"}]}

 RECOVERY (replay existing_log, also copy each record into updated_log):
   put(a,1) -> order: [a]            log += {"key":"a","op":"put","value":1}
   put(b,2) -> order: [b, a]         log += {"key":"b","op":"put","value":2}   (b=newest, a=oldest)

 OPERATIONS:
   get(a) -> hit, value 1; a becomes newest -> order: [a, b]
             results += 1            log += {"hit":true,"key":"a","op":"get"}
   put(c,3)-> insert c; size 3 > capacity 2 -> evict oldest (b)
             order: [c, a]          log += {"key":"c","op":"put","value":3}
   get(b) -> miss (b was evicted)
             results += null         log += {"hit":false,"key":"b","op":"get"}

 OUTPUT (object keys sorted: final_cache < get_results < updated_log;
         final_cache entries sorted by key):
   {"final_cache":[["a",1],["c",3]],
    "get_results":[1,null],
    "updated_log":[ ...5 records above... ]}
*/

class Solution {
    string text;        // the whole input JSON
    size_t pos = 0;     // current parse position

    // Serialize a key->value map back to a JSON object string.
    // Because std::map is sorted, object keys come out alphabetically (canonical).
    string serialize(map<string, string>& fields) {
        string out = "{";
        bool first = true;
        for (auto& field : fields) {
            if (!first) out += ',';
            first = false;
            out += field.first + ':' + field.second;   // field.first already carries quotes
        }
        return out + "}";
    }

    // Parse the object starting at text[pos]=='{' into a {key -> canonicalValue} map.
    map<string, string> parseObject() {
        pos++;                                          // skip '{'
        map<string, string> fields;
        while (text[pos] != '}') {
            string key = parseValue();                  // key is a quoted string token
            pos++;                                      // skip ':'
            fields[key] = parseValue();                 // value (recursively canonical)
            if (text[pos] == ',') pos++;
        }
        pos++;                                          // skip '}'
        return fields;
    }

    // Parse ANY JSON value at text[pos]; return its canonical string form.
    string parseValue() {
        char c = text[pos];
        if (c == '{') { auto obj = parseObject(); return serialize(obj); }
        if (c == '[') {                                 // array: keep element order
            pos++;
            string out = "[";
            bool first = true;
            while (text[pos] != ']') {
                if (!first) out += ',';
                first = false;
                out += parseValue();
                if (text[pos] == ',') pos++;
            }
            pos++;
            return out + "]";
        }
        if (c == '"') {                                 // string: copy verbatim incl. quotes
            size_t end = pos + 1;
            while (text[end] != '"') end++;
            string token = text.substr(pos, end - pos + 1);
            pos = end + 1;
            return token;
        }
        size_t end = pos;                               // number / true / false / null
        while (end < text.size() && text[end] != ',' && text[end] != ']' && text[end] != '}')
            end++;
        string token = text.substr(pos, end - pos);
        pos = end;
        return token;
    }

public:
    string solution(string inputJson) {
        text = inputJson; pos = 0;
        int capacity = 0;

        // LRU store: list front = most-recently-used, back = least-recently-used.
        list<pair<string, string>> order;
        unordered_map<string, list<pair<string, string>>::iterator> slot;

        string logBody, resultBody;                     // joined record / result strings

        // put: insert or refresh a key, evicting the oldest when over capacity.
        auto put = [&](const string& key, const string& value) {
            auto it = slot.find(key);
            if (it != slot.end()) {                      // already cached -> update + promote
                it->second->second = value;
                order.splice(order.begin(), order, it->second);
            } else {                                     // new key -> insert at front
                order.push_front({key, value});
                slot[key] = order.begin();
                if ((int)slot.size() > capacity) {       // evict least-recently-used
                    slot.erase(order.back().first);
                    order.pop_back();
                }
            }
        };
        // get: return value on hit (and promote it), or "" on miss.
        auto get = [&](const string& key) -> string {
            auto it = slot.find(key);
            if (it == slot.end()) return "";
            order.splice(order.begin(), order, it->second);
            return it->second->second;
        };
        auto appendLog = [&](const string& record) {
            if (!logBody.empty()) logBody += ',';
            logBody += record;
        };

        pos++;                                           // skip top-level '{'
        while (text[pos] != '}') {
            string field = parseValue();                 // "capacity" / "existing_log" / "operations"
            pos++;                                        // skip ':'

            if (field == "\"capacity\"") {
                capacity = stoi(parseValue());
            }
            else if (field == "\"existing_log\"") {
                pos++;                                    // skip '['
                while (text[pos] != ']') {
                    if (text[pos] != '{') {               // non-object = malformed tail
                        while (text[pos] != ']') {        // discard it and everything after
                            parseValue();
                            if (text[pos] == ',') pos++;
                        }
                        break;
                    }
                    auto record = parseObject();
                    appendLog(serialize(record));         // recovered record copied verbatim
                    if (record["\"op\""] == "\"put\"")
                        put(record["\"key\""], record["\"value\""]);
                    else
                        get(record["\"key\""]);
                    if (text[pos] == ',') pos++;
                }
                pos++;                                    // skip ']'
            }
            else if (field == "\"operations\"") {
                pos++;                                    // skip '['
                while (text[pos] != ']') {
                    auto record = parseObject();
                    string key = record["\"key\""];
                    if (record["\"op\""] == "\"put\"") {
                        put(key, record["\"value\""]);
                        appendLog(serialize(record));     // canonical put record
                    } else {
                        string value = get(key);
                        bool hit = !value.empty();
                        if (!resultBody.empty()) resultBody += ',';
                        resultBody += hit ? value : "null";
                        appendLog(string("{\"hit\":") + (hit ? "true" : "false")
                                  + ",\"key\":" + key + ",\"op\":\"get\"}");
                    }
                    if (text[pos] == ',') pos++;
                }
                pos++;                                    // skip ']'
            }
            if (text[pos] == ',') pos++;
        }

        // final_cache: dump live entries into a sorted map -> [[key,value],...]
        map<string, string> sorted;
        for (auto& entry : slot) sorted[entry.first] = entry.second->second;
        string cacheBody;
        for (auto& entry : sorted) {
            if (!cacheBody.empty()) cacheBody += ',';
            cacheBody += "[" + entry.first + "," + entry.second + "]";
        }

        return "{\"final_cache\":[" + cacheBody +
               "],\"get_results\":[" + resultBody +
               "],\"updated_log\":[" + logBody + "]}";
    }
};
