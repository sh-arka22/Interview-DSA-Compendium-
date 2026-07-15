#ifndef EMPLOYEE_TIME_TRACKING_SOLUTION_CPP
#define EMPLOYEE_TIME_TRACKING_SOLUTION_CPP

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <optional>
#include <variant>
#include <utility>
using namespace std;

// ── TimeTracker: the engine ───────────────────────────────────────────────────
//
// Exposes a direct, strongly-typed API (add / clockIn / clockOut / totalTime /
// topTime). This is deliberately separate from the operations-log adapter
// below it (see `solution()`), so the core logic is unit-testable on its own
// terms rather than only through the string-log harness.
//
// STATED ASSUMPTIONS (the source problem only gave two worked examples, not a
// full spec — see README.md "Assumptions" for the full list and rationale):
//   1. Operations arrive with non-decreasing timestamps (a forward log, not an
//      out-of-order stream). TOP_TIME/TOTAL_TIME's `ts` argument is accepted
//      for interface symmetry with the other operations but reflects "current
//      state as of this point in the log", not a historical point-in-time query.
//   2. TOTAL_TIME / TOP_TIME count only COMPLETED shifts (a matching
//      CLOCK_IN -> CLOCK_OUT pair). An employee still clocked in contributes
//      nothing until they clock out.
//   3. An employee who has never completed a shift is excluded from TOP_TIME
//      entirely (not shown with a time of 0) — "a completed shift contributes
//      ... to ranking" is read as a membership condition, not just a value.
//   4. TOP_TIME ties break by employee id, ascending (alphabetical).
class TimeTracker {
public:
    // add: registers a new employee. False (no mutation) if id already exists.
    bool add(const string& id, const string& role, long long wage) {
        if (employees_.count(id)) return false;
        employees_.emplace(id, Employee{role, wage});
        return true;
    }

    // clockIn: false (no mutation) if the id is unknown or already clocked in.
    bool clockIn(const string& id, long long ts) {
        auto it = employees_.find(id);
        if (it == employees_.end()) return false;
        Employee& e = it->second;
        if (e.clockedIn) return false;          // invalid transition: double clock-in
        e.clockedIn = true;
        e.clockInTime = ts;
        return true;
    }

    // clockOut: false (no mutation) if the id is unknown or not currently
    // clocked in. On success, folds the completed shift into totalTime and
    // updates the ranking structure in O(log n).
    bool clockOut(const string& id, long long ts) {
        auto it = employees_.find(id);
        if (it == employees_.end()) return false;
        Employee& e = it->second;
        if (!e.clockedIn) return false;         // invalid transition: not clocked in

        long long duration = ts - e.clockInTime;
        if (duration < 0) duration = 0;          // defensive; see assumption (1)

        // Ranking key is (-totalTime, id): remove the stale key BEFORE mutating
        // totalTime, since the set is ordered by that value.
        if (e.completedShifts > 0) ranking_.erase({-e.totalTime, id});
        e.totalTime += duration;
        e.completedShifts += 1;
        e.clockedIn = false;
        ranking_.insert({-e.totalTime, id});

        return true;
    }

    // totalTime: nullopt if the id is unknown. Otherwise the sum of all
    // completed shifts (see assumption 2 — an open shift doesn't count).
    optional<long long> totalTime(const string& id) const {
        auto it = employees_.find(id);
        if (it == employees_.end()) return nullopt;
        return it->second.totalTime;
    }

    // topTime: up to k employees with at least one completed shift, ordered
    // by totalTime descending, ties broken by id ascending. If fewer than k
    // are eligible, returns however many exist. k <= 0 returns empty.
    //
    // ranking_ is already sorted in exactly this order (see the key encoding
    // above), so this is a straight O(k) prefix walk — no per-query sort.
    vector<pair<string, long long>> topTime(int k) const {
        vector<pair<string, long long>> out;
        if (k <= 0) return out;
        out.reserve(min<size_t>(static_cast<size_t>(k), ranking_.size()));
        for (const auto& [negTime, id] : ranking_) {
            if (static_cast<int>(out.size()) >= k) break;
            out.push_back({id, -negTime});
        }
        return out;
    }

private:
    struct Employee {
        string name;
        long long wage = 0;
        bool clockedIn = false;
        long long clockInTime = 0;
        long long totalTime = 0;
        int completedShifts = 0;
    };

    unordered_map<string, Employee> employees_;

    // Ordered by (-totalTime, id): default ascending pair order therefore
    // yields totalTime DESCENDING, with id ASCENDING as the tie-break — the
    // exact TOP_TIME output order, for free, with no per-query sort.
    // Only holds employees with completedShifts > 0 (see assumption 3).
    set<pair<long long, string>> ranking_;
};

// ── Operations-log adapter ────────────────────────────────────────────────────
//
// Each result is one of: bool (ADD/CLOCK_IN/CLOCK_OUT), monostate (TOTAL_TIME
// on an unknown id -> "None"), long long (TOTAL_TIME on a known id), or
// vector<pair<string,long long>> (TOP_TIME).
using Result = variant<bool, monostate, long long, vector<pair<string, long long>>>;

// operations[i] = {op, ts, ...op-specific string args}, e.g.:
//   {"ADD", "0", "a", "dev", "10"}
//   {"CLOCK_IN", "1", "a"}
//   {"CLOCK_OUT", "6", "a"}
//   {"TOTAL_TIME", "6", "a"}
//   {"TOP_TIME", "6", "1"}
vector<Result> solution(const vector<vector<string>>& operations) {
    TimeTracker tracker;
    vector<Result> results;
    results.reserve(operations.size());

    for (const auto& op : operations) {
        const string& cmd = op[0];
        long long ts = stoll(op[1]);

        if (cmd == "ADD") {
            results.push_back(tracker.add(op[2], op[3], stoll(op[4])));

        } else if (cmd == "CLOCK_IN") {
            results.push_back(tracker.clockIn(op[2], ts));

        } else if (cmd == "CLOCK_OUT") {
            results.push_back(tracker.clockOut(op[2], ts));

        } else if (cmd == "TOTAL_TIME") {
            auto t = tracker.totalTime(op[2]);
            if (t.has_value()) results.push_back(*t);
            else results.push_back(monostate{});

        } else if (cmd == "TOP_TIME") {
            results.push_back(tracker.topTime(stoi(op[2])));
        }
    }
    return results;
}

#endif // EMPLOYEE_TIME_TRACKING_SOLUTION_CPP
