#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
using namespace std;

class Solution {
    bool isPosInt(const string& s) {
        if (s.empty()) return false;
        for (char c : s) if (!isdigit(c)) return false;
        return stoi(s) > 0;
    }

    bool validCatalogFields(const map<string, string>& cat) {
        for (const char* f : {"availableQuantity", "minQuantity", "maxQuantity"}) {
            auto it = cat.find(f);
            if (it == cat.end()) return false;
            for (char c : it->second) if (!isdigit(c)) return false;
        }
        return true;
    }

    // Serialize errors to JSON array string.
    // Numeric-only values (index) are emitted without quotes to match expected output.
    string toJson(const vector<map<string, string>>& errs) {
        if (errs.empty()) return "[]";
        string out = "[";
        for (const auto& e : errs) {
            out += "{";
            bool first = true;
            for (const string& k : {"index", "itemId", "code"}) {
                auto it = e.find(k);
                if (it == e.end()) continue;
                if (!first) out += ", ";
                const string& v = it->second;
                bool isNum = !v.empty() && all_of(v.begin(), v.end(), ::isdigit);
                out += "\"" + k + "\": " + (isNum ? v : "\"" + v + "\"");
                first = false;
            }
            out += "}, ";
        }
        out.pop_back(); out.pop_back(); // trim trailing ", "
        return out + "]";
    }

public:
    map<string, string> solution(vector<map<string, string>> cart,vector<map<string, string>> catalog) {
        vector<map<string, string>> errors;

        if (cart.empty()) {
            errors.push_back({{"code", "EMPTY_CART"}});
            return {{"isValid", "false"}, {"errors", toJson(errors)}};
        }

        unordered_map<string, map<string, string>> catMap;
        for (auto& item : catalog) catMap[item["itemId"]] = item;

        unordered_map<string, int> totals;
        vector<string>             order;
        unordered_set<string>      seen;

        // ── Pass 1: per-line checks ───────────────────────────────────────────
        for (int i = 0; i < (int)cart.size(); i++) {
            auto& line = cart[i];
            string idx = to_string(i);

            auto idIt = line.find("itemId");
            if (idIt == line.end() || idIt->second.empty()) {
                errors.push_back({{"index", idx}, {"code", "MISSING_ITEM_ID"}}); continue;
            }
            const string& itemId = idIt->second;

            auto qIt = line.find("quantity");
            if (qIt == line.end()) {
                errors.push_back({{"index", idx}, {"itemId", itemId}, {"code", "MISSING_QUANTITY"}}); continue;
            }
            if (!isPosInt(qIt->second)) {
                errors.push_back({{"index", idx}, {"code", "INVALID_QUANTITY"}}); continue;
            }
            int qty = stoi(qIt->second);

            auto catIt = catMap.find(itemId);
            if (catIt == catMap.end()) {
                errors.push_back({{"index", idx}, {"itemId", itemId}, {"code", "ITEM_NOT_FOUND"}}); continue;
            }
            if (!validCatalogFields(catIt->second)) {
                errors.push_back({{"index", idx}, {"itemId", itemId}, {"code", "INVALID_CATALOG_ITEM"}}); continue;
            }

            totals[itemId] += qty;
            if (seen.insert(itemId).second) order.push_back(itemId);
        }

        // ── Pass 2: aggregate checks in first-seen order ──────────────────────
        for (const auto& itemId : order) {
            const auto& cat = catMap[itemId];
            int total = totals[itemId];
            int avail = stoi(cat.at("availableQuantity"));
            int minQ  = stoi(cat.at("minQuantity"));
            int maxQ  = stoi(cat.at("maxQuantity"));

            if      (total > avail) errors.push_back({{"itemId", itemId}, {"code", "EXCEEDS_AVAILABLE"}});
            else if (total < minQ)  errors.push_back({{"itemId", itemId}, {"code", "BELOW_MIN_QUANTITY"}});
            else if (total > maxQ)  errors.push_back({{"itemId", itemId}, {"code", "EXCEEDS_MAX_QUANTITY"}});
        }

        return {{"isValid", errors.empty() ? "true" : "false"}, {"errors", toJson(errors)}};
    }
};
