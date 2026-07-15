// Part 1 — Minimum Bills and Coins to Make Change (Easy)
// Denominations (cents, descending): $20, $10, $5, $1, $0.25, $0.10, $0.05, $0.01
//
// APPROACH: Greedy.
// With this specific denomination set and unlimited stock, always taking as many
// of the largest remaining denomination as fit is provably optimal (see
// ALGORITHM.md for the exchange-argument proof sketch). That is what makes this
// an "Easy" — the moment stock becomes finite (Part 2), greedy stops being safe.
//
// KEY DECISION: convert dollars -> integer cents FIRST.
// IEEE-754 double cannot represent most cent values exactly (0.1 in binary is a
// repeating fraction), so comparisons/subtractions done in dollar-space can drift
// by fractions of a cent and silently miscount. Doing round(amount * 100) once,
// up front, moves all subsequent arithmetic into exact integers.

#include <vector>
#include <cmath>
using namespace std;

class Solution {
public:
    // Return {total, c2000, c1000, c500, c100, c25, c10, c5, c1}
    vector<int> minPieces(double amount) {
        long long cents = llround(amount * 100.0);

        static const int denomsCents[8] = {2000, 1000, 500, 100, 25, 10, 5, 1};

        vector<int> counts(8, 0);
        int total = 0;

        for (int i = 0; i < 8 && cents > 0; ++i) {
            int take = static_cast<int>(cents / denomsCents[i]);
            counts[i] = take;
            total += take;
            cents -= static_cast<long long>(take) * denomsCents[i];

            // Early exit (follow-up): once cents hits 0 there's nothing left to
            // place. The remaining denominations would all take 0 units anyway
            // (counts[] is already zero-initialized), so breaking here is purely
            // an efficiency win, not a correctness requirement — but it's the
            // idiom an interviewer wants to see named explicitly.
            if (cents == 0) break;
        }

        vector<int> result(9);
        result[0] = total;
        for (int i = 0; i < 8; ++i) result[i + 1] = counts[i];
        return result;
    }
};

// Complexity: O(1) time and O(1) extra space — exactly 8 fixed denominations,
// independent of the size of `amount`.
