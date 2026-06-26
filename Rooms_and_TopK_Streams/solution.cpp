#ifndef ROOMS_TOPK_SOLUTION_CPP
#define ROOMS_TOPK_SOLUTION_CPP

#include <vector>
#include <algorithm>
#include <queue>
#include <utility>

// ── Problem 1: Minimum Concurrent Meeting Rooms ──────────────────────────────
//
// Two-pointer approach.
//
// Sort starts[] and ends[] independently. For each start[i]:
//   - If start[i] < ends[e]: no room has freed yet; allocate a new one (++rooms).
//   - Otherwise (start[i] >= ends[e]): the earliest-ending ongoing meeting freed
//     its room; reuse it (++e, rooms unchanged).
//
// The strict '<' is the key boundary condition: end-at-t frees the room for a
// meeting starting at t, so they do NOT need two rooms.
//
// 'rooms' only ever increases (freed rooms are immediately reused, not returned
// to a pool). maxRooms is the peak simultaneous occupancy — the answer.
//
// Time: O(N log N)  |  Space: O(N)

int minMeetingRooms(const std::vector<std::pair<int, int>>& intervals) {
    if (intervals.empty()) return 0;
    const int n = static_cast<int>(intervals.size());

    std::vector<int> starts(n), ends(n);
    for (int i = 0; i < n; ++i) {
        starts[i] = intervals[i].first;
        ends[i]   = intervals[i].second;
    }
    std::sort(starts.begin(), starts.end());
    std::sort(ends.begin(), ends.end());

    int rooms = 0, maxRooms = 0, e = 0;
    for (int i = 0; i < n; ++i) {
        if (starts[i] < ends[e]) {
            ++rooms;
        } else {
            ++e; // earliest room freed and immediately reused; net count unchanged
        }
        maxRooms = std::max(maxRooms, rooms);
    }
    return maxRooms;
}

// Min-heap alternative — kept for follow-up discussion.
// Sorts by start time; heap holds end times of active meetings.
// After processing all meetings, heap.size() == minimum rooms needed.
// Same O(N log N) time/space, but slightly higher constant from heap overhead.
int minMeetingRoomsHeap(std::vector<std::pair<int, int>> intervals) {
    if (intervals.empty()) return 0;
    std::sort(intervals.begin(), intervals.end()); // sort by start time

    std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
    for (const auto& [start, end] : intervals) {
        if (!pq.empty() && pq.top() <= start)
            pq.pop(); // earliest-ending meeting freed its room
        pq.push(end);
    }
    return static_cast<int>(pq.size());
}

// ── Problem 2: Top-K Stream ──────────────────────────────────────────────────
//
// Maintains a fixed-size min-heap (by "weakness") of the current top-k items.
// The weakest item sits at the top so we can compare and evict it in O(1).
//
// Weakness ordering (weakest = top of heap = first to be evicted):
//   - Lower score  → weaker
//   - Same score, larger itemId → weaker  (smaller id wins on tie)
//
// Heap pair layout: (score, itemId).
//
// comp(a, b) = true  means  a has LOWER priority (is STRONGER → sinks deeper).
// This gives us a "weakest-at-top" structure using C++'s default max-heap.
//
// add:     O(log k)
// getTopK: O(k log k)  — copy heap + sort
// Space:   O(k)

class TopKStream {
    const int k_;

    struct WeakerAtTop {
        bool operator()(const std::pair<int, int>& a,
                        const std::pair<int, int>& b) const {
            // Returns true if 'a' is STRONGER than 'b' (lower priority → sinks).
            if (a.first != b.first) return a.first > b.first;  // higher score = stronger
            return a.second < b.second;                         // smaller id   = stronger
        }
    };

    std::priority_queue<std::pair<int, int>,
                        std::vector<std::pair<int, int>>,
                        WeakerAtTop> heap_; // (score, itemId)

    // True if (scoreA, idA) should displace (scoreB, idB) as the heap top.
    static bool strongerThan(int scoreA, int idA, int scoreB, int idB) {
        if (scoreA != scoreB) return scoreA > scoreB;
        return idA < idB; // smaller id wins on equal score
    }

public:
    explicit TopKStream(int k) : k_(k) {}

    void add(int itemId, int score) {
        if (k_ == 0) return;

        if (static_cast<int>(heap_.size()) < k_) {
            heap_.push({score, itemId});
            return;
        }

        // Heap is full: evict the weakest only if the new item is strictly stronger.
        const auto& [topScore, topId] = heap_.top();
        if (strongerThan(score, itemId, topScore, topId)) {
            heap_.pop();
            heap_.push({score, itemId});
        }
        // If not strictly stronger, discard — existing item is at least as good.
    }

    // Returns at most k items sorted by score DESC, then itemId ASC.
    // Pairs are (itemId, score).
    std::vector<std::pair<int, int>> getTopK() const {
        // priority_queue has no iterators — copy and drain.
        auto tmp = heap_;
        std::vector<std::pair<int, int>> items;
        items.reserve(tmp.size());
        while (!tmp.empty()) {
            items.push_back(tmp.top()); // (score, itemId)
            tmp.pop();
        }
        // Sort: score DESC, then itemId ASC.
        std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
            if (a.first != b.first) return a.first > b.first;
            return a.second < b.second;
        });
        std::vector<std::pair<int, int>> result;
        result.reserve(items.size());
        for (const auto& [sc, id] : items) result.push_back({id, sc});
        return result;
    }

    int size() const { return static_cast<int>(heap_.size()); }
};

#endif // ROOMS_TOPK_SOLUTION_CPP
