#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;

class Solution {
  // Check if string represents a positive integer (digits only, value > 0)
  bool isPosInt(const string &s) {
    if (s.empty())
      return false;
    for (char c : s)
      if (c < '0' || c > '9')
        return false;
    int val = 0;
    for (char c : s)
      val = val * 10 + (c - '0');
    return val > 0;
  }

  int toInt(const string &s) {
    int val = 0;
    for (char c : s)
      val = val * 10 + (c - '0');
    return val;
  }

  // Check catalog has the three required fields and they are non-negative
  // integers
  bool validCatalogFields(const map<string, string> &cat) {
    for (const string &f :
         {"availableQuantity", "minQuantity", "maxQuantity"}) {
      auto it = cat.find(f);
      if (it == cat.end())
        return false;
      const string &v = it->second;
      if (v.empty())
        return false;
      for (char c : v)
        if (c < '0' || c > '9')
          return false;
    }
    return true;
  }

  // Serialize a single error object as JSON
  // Keys are sorted alphabetically (map iteration order)
  // "index" values are numeric, everything else is a quoted string
  string errorToJson(const map<string, string> &err) {
    string s = "{";
    bool first = true;
    for (auto &[k, v] : err) {
      if (!first)
        s += ", ";
      first = false;
      s += "\"" + k + "\": ";
      if (k == "index")
        s += v; // numeric, no quotes
      else
        s += "\"" + v + "\""; // string, with quotes
    }
    s += "}";
    return s;
  }

  string errorsToJson(const vector<map<string, string>> &errors) {
    string s = "[";
    for (int i = 0; i < (int)errors.size(); i++) {
      if (i > 0)
        s += ", ";
      s += errorToJson(errors[i]);
    }
    s += "]";
    return s;
  }

public:
  map<string, string> solution(vector<map<string, string>> cart,
                               vector<map<string, string>> catalog) {
    map<string, string> result;
    vector<map<string, string>> errors;

    // Rule 1: Cart must not be empty
    if (cart.empty()) {
      errors.push_back({{"code", "EMPTY_CART"}});
      result["isValid"] = "false";
      result["errors"] = errorsToJson(errors);
      return result;
    }

    // Build catalog lookup: itemId → catalog entry
    unordered_map<string, map<string, string>> catMap;
    for (auto &item : catalog)
      catMap[item["itemId"]] = item;

    unordered_map<string, int> totals;
    vector<string> order;
    unordered_set<string> seen;

    // ── Pass 1: per-line validation (errors in cart order) ──
    for (int i = 0; i < (int)cart.size(); i++) {
      auto &line = cart[i];
      string idx = to_string(i);

      // Rule 2: must have itemId
      auto idIt = line.find("itemId");
      if (idIt == line.end() || idIt->second.empty()) {
        errors.push_back({{"code", "MISSING_ITEM_ID"}, {"index", idx}});
        continue;
      }
      string itemId = idIt->second;

      // Rule 3: must have quantity
      auto qIt = line.find("quantity");
      if (qIt == line.end()) {
        errors.push_back(
            {{"code", "MISSING_QUANTITY"}, {"index", idx}, {"itemId", itemId}});
        continue;
      }

      // Rule 4: quantity must be a positive integer
      if (!isPosInt(qIt->second)) {
        errors.push_back({{"code", "INVALID_QUANTITY"}, {"index", idx}});
        continue;
      }
      int qty = toInt(qIt->second);

      // Rule 5: item must exist in catalog
      auto catIt = catMap.find(itemId);
      if (catIt == catMap.end()) {
        errors.push_back(
            {{"code", "ITEM_NOT_FOUND"}, {"index", idx}, {"itemId", itemId}});
        continue;
      }

      // Rule 7: catalog fields must exist and be integers
      if (!validCatalogFields(catIt->second)) {
        errors.push_back({{"code", "INVALID_CATALOG_ITEM"},
                          {"index", idx},
                          {"itemId", itemId}});
        continue;
      }

      // Rule 6: accumulate quantities for duplicate itemIds
      totals[itemId] += qty;
      if (seen.insert(itemId).second)
        order.push_back(itemId);
    }

    // ── Pass 2: aggregate checks in first-seen order ──
    for (auto &itemId : order) {
      auto &cat = catMap[itemId];
      int total = totals[itemId];
      int avail = toInt(cat["availableQuantity"]);
      int minQ = toInt(cat["minQuantity"]);
      int maxQ = toInt(cat["maxQuantity"]);

      // Rule 8
      if (total > avail)
        errors.push_back({{"code", "EXCEEDS_AVAILABLE"}, {"itemId", itemId}});
      // Rule 9
      else if (total < minQ)
        errors.push_back({{"code", "BELOW_MIN_QUANTITY"}, {"itemId", itemId}});
      // Rule 10
      else if (total > maxQ)
        errors.push_back(
            {{"code", "EXCEEDS_MAX_QUANTITY"}, {"itemId", itemId}});
    }

    result["isValid"] = errors.empty() ? "true" : "false";
    result["errors"] = errorsToJson(errors);
    return result;
  }
};
