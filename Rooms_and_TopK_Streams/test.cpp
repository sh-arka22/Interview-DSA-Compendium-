// Compile: g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <cassert>
#include <iostream>
#include <string>

using IV = std::vector<std::pair<int, int>>;
using PV = std::vector<std::pair<int, int>>;

static void check(const std::string& name, bool cond) {
    if (!cond) { std::cerr << "FAIL  " << name << "\n"; std::exit(1); }
    std::cout << "PASS  " << name << "\n";
}

// Run both two-pointer and min-heap; assert both equal 'expected'.
static void checkRooms(const std::string& tag, const IV& iv, int expected) {
    IV copy = iv;
    check(tag + " (two-ptr)", minMeetingRooms(iv)     == expected);
    check(tag + " (heap)",    minMeetingRoomsHeap(copy) == expected);
}

// ── Problem 1 Tests ───────────────────────────────────────────────────────────

void test_rooms() {
    // Trivial cases
    checkRooms("empty",         {},                       0);
    checkRooms("single",        {{1, 3}},                 1);

    // Problem example
    checkRooms("example",       {{0,30},{5,10},{15,20}},  2);

    // ── Boundary condition: end-at-t frees room for start-at-t ──
    // The most common wrong answer on this problem is returning 2 here.
    checkRooms("back-to-back",  {{0,5},{5,10}},           1);
    checkRooms("chain-3",       {{0,5},{5,10},{10,15}},   1); // all reuse same room
    checkRooms("neg-touch",     {{-10,-5},{-5,0}},        1); // same rule, negative times

    // ── Multiple simultaneous meetings ──
    checkRooms("all-same",      {{3,7},{3,7},{3,7}},      3);
    checkRooms("duplicates",    {{2,5},{2,5}},             2);

    // ── No overlap ──
    // Gap between: end < next start
    checkRooms("no-overlap",    {{0,3},{4,7},{8,12}},     1);

    // ── Nesting ──
    checkRooms("all-nested",    {{1,10},{2,8},{3,6}},     3);

    // ── Two groups: each group needs 2 rooms, groups don't overlap ──
    // [0,2] and [0,2] overlap; [3,5] and [3,5] overlap; no cross-group overlap.
    checkRooms("two-groups",    {{0,2},{0,2},{3,5},{3,5}}, 2);

    // ── Complex: 4 meetings, peak is 3 ──
    // [0,10] and [0,3] run together from t=0; [3,7] reuses [0,3]'s room at t=3;
    // [5,12] starts at t=5 when [0,10] and [3,7] are both active → 3rd room.
    checkRooms("complex",       {{0,10},{0,3},{3,7},{5,12}}, 3);

    // ── Negative time values ──
    // [-10,-5] and [-7,-3] overlap in [-7,-5] → 2 rooms.
    checkRooms("negative",      {{-10,-5},{-7,-3}},       2);

    // ── Large time values (algorithm must not depend on time range) ──
    checkRooms("large-times",   {{0,1000000000},{500000000,999999999}}, 2);

    // ── Two freed rooms, only one new meeting ──
    // Both [0,1] meetings end before [3,4] starts; [3,4] reuses only one freed room.
    // Peak = 2 (during t=0..1). Not 1 — the peak already happened.
    checkRooms("two-freed-one-new", {{0,1},{0,1},{3,4}},  2);
}

// ── Problem 2 Tests ───────────────────────────────────────────────────────────

void test_topk() {
    // k = 0: always empty regardless of inserts
    {
        TopKStream t(0);
        t.add(1, 100);
        t.add(2, 200);
        check("k0: always empty", t.getTopK().empty());
    }

    // k = 1, single item
    {
        TopKStream t(1);
        t.add(42, 7);
        PV r = t.getTopK();
        check("k1-single: size", r.size() == 1);
        check("k1-single: item", r[0] == std::make_pair(42, 7));
    }

    // Fewer than k items seen → return all seen
    {
        TopKStream t(5);
        t.add(3, 9);
        PV r = t.getTopK();
        check("k>n: size",  r.size() == 1);
        check("k>n: item",  r[0] == std::make_pair(3, 9));
    }

    // Basic top-2 by score
    {
        TopKStream t(2);
        t.add(1, 5);
        t.add(2, 9);
        t.add(3, 3);  // weakest; should be evicted
        PV r = t.getTopK();
        check("top2-basic: size",   r.size() == 2);
        check("top2-basic: first",  r[0] == std::make_pair(2, 9));
        check("top2-basic: second", r[1] == std::make_pair(1, 5));
    }

    // ── Tie-breaking: same score → smaller itemId wins ──
    // Items: (id=3,score=10), (id=1,score=10), (id=2,score=5).  k=2.
    // score=5 is weakest; top-2 = {(1,10),(3,10)}.
    {
        TopKStream t(2);
        t.add(3, 10);
        t.add(1, 10);
        t.add(2,  5);
        PV r = t.getTopK();
        check("tie-basic: size",   r.size() == 2);
        check("tie-basic: first",  r[0] == std::make_pair(1, 10));
        check("tie-basic: second", r[1] == std::make_pair(3, 10));
    }

    // ── Tie-breaking at the boundary of the heap ──
    // k=2.  Add (id=1,10) and (id=3,10).  Heap top = (10,3) (weaker: larger id).
    // Now add (id=5,10): id=5 > id=3 → NOT stronger → rejected.
    // Now add (id=2,10): id=2 < id=3 → stronger   → evicts (id=3), top-2 = {1,2}.
    {
        TopKStream t(2);
        t.add(1, 10);
        t.add(3, 10);
        t.add(5, 10); // rejected: id=5 > current weakest id=3
        PV r = t.getTopK();
        check("tie-boundary-reject: still {1,3}", r.size() == 2 &&
              r[0] == std::make_pair(1,10) && r[1] == std::make_pair(3,10));

        t.add(2, 10); // admitted: id=2 < id=3
        PV r2 = t.getTopK();
        check("tie-boundary-admit: now {1,2}", r2.size() == 2 &&
              r2[0] == std::make_pair(1,10) && r2[1] == std::make_pair(2,10));
    }

    // ── Many ties: k=3, ids 5→1 inserted; top-3 should be {1,2,3} ──
    {
        TopKStream t(3);
        for (int id = 5; id >= 1; --id) t.add(id, 7);
        PV r = t.getTopK();
        check("many-ties: size", r.size() == 3);
        check("many-ties: id1",  r[0] == std::make_pair(1, 7));
        check("many-ties: id2",  r[1] == std::make_pair(2, 7));
        check("many-ties: id3",  r[2] == std::make_pair(3, 7));
    }

    // ── Negative scores ──
    // Items: (id=1,−10), (id=2,−3), (id=3,−5).  k=2.
    // Score order: −3 > −5 > −10.  Top-2 = {(2,−3),(3,−5)}.
    {
        TopKStream t(2);
        t.add(1, -10);
        t.add(2,  -3);
        t.add(3,  -5);
        PV r = t.getTopK();
        check("neg-scores: size",   r.size() == 2);
        check("neg-scores: first",  r[0] == std::make_pair(2, -3));
        check("neg-scores: second", r[1] == std::make_pair(3, -5));
    }

    // ── k = 1: always keep the single best; tie goes to smaller id ──
    // (id=10,5) → (id=2,20) → (id=7,20): id=7 NOT stronger (7 > 2) → keep id=2.
    {
        TopKStream t(1);
        t.add(10, 5);
        t.add(2,  20);
        t.add(7,  20); // same score as id=2 but larger id → rejected
        t.add(1,  15); // lower score than 20 → rejected
        PV r = t.getTopK();
        check("k1-best: size",  r.size() == 1);
        check("k1-best: item",  r[0] == std::make_pair(2, 20));
    }

    // ── Exact duplicate: (id=5, score=10) added twice ──
    // Second is NOT strictly stronger (same (score,id)) → discarded.
    {
        TopKStream t(1);
        t.add(5, 10);
        t.add(5, 10);
        check("duplicate: still size 1", t.size() == 1);
        check("duplicate: correct item", t.getTopK()[0] == std::make_pair(5, 10));
    }

    // ── k > stream length → return all N seen items ──
    // 3 items, k=5.  Returned sorted: (2,8),(3,8),(1,5).
    {
        TopKStream t(5);
        t.add(2, 8);
        t.add(1, 5);
        t.add(3, 8);
        PV r = t.getTopK();
        check("k>n-multi: size",   r.size() == 3);
        check("k>n-multi: first",  r[0] == std::make_pair(2, 8));
        check("k>n-multi: second", r[1] == std::make_pair(3, 8));
        check("k>n-multi: third",  r[2] == std::make_pair(1, 5));
    }

    // ── Mixed: score update scenario (basic version — old entry stays in heap) ──
    // id=1 appears with score=5, then score=50.  With k=2, id=1 should have score=50.
    // Basic version: treats second add as a new item; evicts weakest if stronger.
    // After add(1,5) and add(2,9): heap = {(5,1),(9,2)}, top=(5,1).
    // add(1,50): 50 > 5 → stronger → evict (5,1), push (50,1).
    // Result: {(1,50),(2,9)}.
    {
        TopKStream t(2);
        t.add(1, 5);
        t.add(2, 9);
        t.add(1, 50); // "update" in basic version: treated as new stronger item
        PV r = t.getTopK();
        check("score-update-basic: size",   r.size() == 2);
        check("score-update-basic: first",  r[0] == std::make_pair(1, 50));
        check("score-update-basic: second", r[1] == std::make_pair(2,  9));
    }
}

// ── Main ─────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "── Problem 1: Meeting Rooms ─────────────────────────────\n";
    test_rooms();
    std::cout << "\n── Problem 2: Top-K Stream ──────────────────────────────\n";
    test_topk();
    std::cout << "\nAll tests passed.\n";
    return 0;
}
