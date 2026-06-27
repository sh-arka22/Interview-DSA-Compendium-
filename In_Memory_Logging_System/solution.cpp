#ifndef IN_MEMORY_LOGGING_SOLUTION_CPP
#define IN_MEMORY_LOGGING_SOLUTION_CPP

#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <memory>

// ── Handler pipeline ──────────────────────────────────────────────────────────
//
// Each handler transforms a log message.  New handler types only require
// subclassing — the dispatch loop in Logger never changes.

struct Handler {
    virtual std::string apply(const std::string& msg) const = 0;
    virtual ~Handler() = default;
};

struct UpperHandler : Handler {
    std::string apply(const std::string& msg) const override {
        std::string r = msg;
        std::transform(r.begin(), r.end(), r.begin(),
                       [](unsigned char c){ return std::toupper(c); });
        return r;
    }
};

struct PrefixHandler : Handler {
    std::string text_;
    explicit PrefixHandler(std::string t) : text_(std::move(t)) {}
    std::string apply(const std::string& msg) const override { return text_ + msg; }
};

struct SuffixHandler : Handler {
    std::string text_;
    explicit SuffixHandler(std::string t) : text_(std::move(t)) {}
    std::string apply(const std::string& msg) const override { return msg + text_; }
};

// ── Logger ────────────────────────────────────────────────────────────────────
//
// Maintains:
//   1. An ordered handler pipeline applied to every incoming log message.
//   2. A sequential store of transformed messages; log id = vector index + 1.
//   3. An inverted index: word → ascending list of log ids (no duplicates per word).
//      Built incrementally on each add_log — Part 2 optimised search.
//
// Complexity:
//   add_handler : O(1)
//   add_log     : O(M·H + W)   M = message chars, H = handlers, W = unique words
//   get_logs    : O(L)         L = total logs stored
//   search      : O(K)         K = matching logs  (via inverted index)
//   searchLinear: O(L·W)       Part 1 baseline — produces identical output

class Logger {
    std::vector<std::unique_ptr<Handler>> handlers_;
    std::vector<std::string>              logs_;   // logs_[i] → log id i+1
    std::unordered_map<std::string, std::vector<int>> index_; // word → [id, ...]

    std::string applyHandlers(const std::string& msg) const {
        std::string r = msg;
        for (const auto& h : handlers_) r = h->apply(r);
        return r;
    }

    // Tokenize transformed message by whitespace; add this log's id once per
    // unique word.  A 'seen' set prevents duplicate ids for repeated words.
    void indexLog(const std::string& msg, int id) {
        std::unordered_set<std::string> seen;
        std::istringstream iss(msg);
        std::string word;
        while (iss >> word) {
            if (seen.insert(word).second)    // first occurrence in this log
                index_[word].push_back(id);  // ids arrive in ascending order
        }
    }

public:
    void addHandler(const std::string& type, const std::string& text = "") {
        if      (type == "upper")  handlers_.push_back(std::make_unique<UpperHandler>());
        else if (type == "prefix") handlers_.push_back(std::make_unique<PrefixHandler>(text));
        else if (type == "suffix") handlers_.push_back(std::make_unique<SuffixHandler>(text));
    }

    void addLog(const std::string& rawMsg) {
        std::string transformed = applyHandlers(rawMsg);
        int id = static_cast<int>(logs_.size()) + 1;
        logs_.push_back(transformed);
        indexLog(transformed, id);
    }

    std::vector<std::string> getLogs() const {
        std::vector<std::string> result;
        result.reserve(logs_.size());
        for (int i = 0; i < static_cast<int>(logs_.size()); ++i)
            result.push_back(std::to_string(i + 1) + ":" + logs_[i]);
        return result;
    }

    // Inverted-index search (Part 2).
    // Empty keyword → [].  Missing keyword → [].
    // Returns logs in insertion order (index lists are already sorted by id).
    std::vector<std::string> search(const std::string& keyword) const {
        if (keyword.empty()) return {};
        auto it = index_.find(keyword);
        if (it == index_.end()) return {};
        std::vector<std::string> result;
        result.reserve(it->second.size());
        for (int id : it->second)
            result.push_back(std::to_string(id) + ":" + logs_[id - 1]);
        return result;
    }

    // Linear-scan search (Part 1 baseline).  Same output as search().
    // Breaks after the first match in each log to avoid duplicate results.
    std::vector<std::string> searchLinear(const std::string& keyword) const {
        if (keyword.empty()) return {};
        std::vector<std::string> result;
        for (int i = 0; i < static_cast<int>(logs_.size()); ++i) {
            std::istringstream iss(logs_[i]);
            std::string word;
            while (iss >> word) {
                if (word == keyword) {
                    result.push_back(std::to_string(i + 1) + ":" + logs_[i]);
                    break; // this log already matched; don't add it again
                }
            }
        }
        return result;
    }
};

// ── Dispatcher (platform / contest entry point) ───────────────────────────────
// Only 'get_logs' and 'search' append to results; other operations are silent.

std::vector<std::vector<std::string>> solution(
    std::vector<std::vector<std::string>> operations
) {
    Logger logger;
    std::vector<std::vector<std::string>> results;

    for (const auto& op : operations) {
        if (op.empty()) continue;
        const std::string& cmd = op[0];

        if (cmd == "add_handler") {
            // op[1] = type; op[2] = text (absent for 'upper')
            logger.addHandler(op[1], op.size() >= 3 ? op[2] : "");
        } else if (cmd == "add_log") {
            logger.addLog(op.size() >= 2 ? op[1] : "");
        } else if (cmd == "get_logs") {
            results.push_back(logger.getLogs());
        } else if (cmd == "search") {
            results.push_back(logger.search(op.size() >= 2 ? op[1] : ""));
        }
    }

    return results;
}

#endif // IN_MEMORY_LOGGING_SOLUTION_CPP
