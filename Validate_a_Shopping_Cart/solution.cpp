#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
using namespace std;

class Solution {
    // Accepts "2", "2.0", "2.00" (platform serialises Python ints as float strings).
    // Rejects "0", "-1", "1.5", "True", "".
    bool isPosInt(const string& s) {
        if (s.empty()) return false;
        try {
            size_t pos;
            int v = stoi(s, &pos);   // stops at '.'; doesn't throw for "2.0"
            if (v <= 0) return false;
            if (pos == s.size()) return true;
            if (s[pos] != '.') return false;
            for (size_t i = pos + 1; i < s.size(); i++)
                if (s[i] != '0') return false;   // "2.5" rejected; "2.0" accepted
            return true;
        } catch (...) { return false; }
    }

    int toInt(const string& s) {
        try { return stoi(s); } catch (...) { return 0; }
    }

    bool validCatalogFields(const map<string, string>& cat) {
        for (const char* f : {"availableQuantity", "minQuantity", "maxQuantity"}) {
            auto it = cat.find(f);
            if (it == cat.end() || it->second.empty()) return false;
            const string& v = it->second;
            try {
                size_t pos;
                stoi(v, &pos);
                if (pos == v.size()) continue;
                if (v[pos] != '.') return false;
                for (size_t i = pos + 1; i < v.size(); i++)
                    if (v[i] != '0') return false;
            } catch (...) { return false; }
        }
        return true;
    }

    string errorToJson(const map<string, string>& err) {
        string s = "{";
        bool first = true;
        for (auto& [k, v] : err) {
            if (!first) s += ", ";
            first = false;
            s += "\"" + k + "\": ";
            s += (k == "index") ? v : "\"" + v + "\"";
        }
        return s + "}";
    }

    string errorsToJson(const vector<map<string, string>>& errors) {
        string s = "[";
        for (int i = 0; i < (int)errors.size(); i++) {
            if (i) s += ", ";
            s += errorToJson(errors[i]);
        }
        return s + "]";
    }

public:
    map<string, string> solution(vector<map<string, string>> cart,
                                 vector<map<string, string>> catalog) {
        vector<map<string, string>> errors;

        if (cart.empty()) {
            errors.push_back({{"code", "EMPTY_CART"}});
            return {{"isValid", "false"}, {"errors", errorsToJson(errors)}};
        }

        unordered_map<string, map<string, string>> catMap;
        for (auto& item : catalog) catMap[item["itemId"]] = item;

        unordered_map<string, int> totals;
        vector<string>             order;
        unordered_set<string>      seen;

        for (int i = 0; i < (int)cart.size(); i++) {
            auto& line = cart[i];
            string idx = to_string(i);

            auto idIt = line.find("itemId");
            if (idIt == line.end() || idIt->second.empty()) {
                errors.push_back({{"code", "MISSING_ITEM_ID"}, {"index", idx}}); continue;
            }
            string itemId = idIt->second;

            auto qIt = line.find("quantity");
            if (qIt == line.end()) {
                errors.push_back({{"code", "MISSING_QUANTITY"}, {"index", idx}, {"itemId", itemId}}); continue;
            }
            if (!isPosInt(qIt->second)) {
                errors.push_back({{"code", "INVALID_QUANTITY"}, {"index", idx}}); continue;
            }
            int qty = toInt(qIt->second);

            auto catIt = catMap.find(itemId);
            if (catIt == catMap.end()) {
                errors.push_back({{"code", "ITEM_NOT_FOUND"}, {"index", idx}, {"itemId", itemId}}); continue;
            }
            if (!validCatalogFields(catIt->second)) {
                errors.push_back({{"code", "INVALID_CATALOG_ITEM"}, {"index", idx}, {"itemId", itemId}}); continue;
            }

            totals[itemId] += qty;
            if (seen.insert(itemId).second) order.push_back(itemId);
        }

        for (const auto& itemId : order) {
            const auto& cat = catMap[itemId];
            int total = totals[itemId];
            int avail = toInt(cat.at("availableQuantity"));
            int minQ  = toInt(cat.at("minQuantity"));
            int maxQ  = toInt(cat.at("maxQuantity"));

            if      (total > avail) errors.push_back({{"code", "EXCEEDS_AVAILABLE"},    {"itemId", itemId}});
            else if (total < minQ)  errors.push_back({{"code", "BELOW_MIN_QUANTITY"},   {"itemId", itemId}});
            else if (total > maxQ)  errors.push_back({{"code", "EXCEEDS_MAX_QUANTITY"}, {"itemId", itemId}});
        }

        return {{"isValid", errors.empty() ? "true" : "false"}, {"errors", errorsToJson(errors)}};
    }
};
