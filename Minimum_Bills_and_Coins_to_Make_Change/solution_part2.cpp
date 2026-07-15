// Part 2 — Minimum Bills and Coins with Limited Inventory (Medium)
// Denominations (cents, descending): $20, $10, $5, $1, $0.25, $0.10, $0.05, $0.01
// inventory[i] = units of denominations[i] available in the drawer.
//
// WHY GREEDY BREAKS: greedy's optimality proof for Part 1 relies on unlimited
// supply — the "exchange argument" assumes you can always swap k small coins for
// one big one. Finite stock removes that assumption. Classic counterexample:
// amount = $0.30, inventory = {one $0.25, three $0.10}. Greedy grabs the quarter
// first, strands $0.05 with no nickel/penny in stock, and reports failure — yet
// three dimes make exact change in 3 pieces. Greedy can be both wrong AND
// unnecessarily infeasible.
//
// APPROACH: bounded knapsack via binary splitting, solved with 0/1-knapsack DP.
//   dp[j] = minimum pieces to make exactly j cents, using stock consumed so far.
//   dp[0] = 0, everything else starts at "infinity" (infeasible).
//
// Treating each of a denomination's `inventory[i]` individual coins as its own
// 0/1 knapsack item works but costs O(cents * sum(inventory)) — too slow if a
// denomination has thousands in the drawer. Binary splitting groups a stock of
// count `c` into parcels of size 1, 2, 4, 8, ..., with a final remainder parcel,
// i.e. O(log c) parcels instead of c individual coins. Any achievable quantity
// from 0..c can be built by summing a subset of those parcels — exactly the way
// any integer in [0, c] is expressible as a sum of a subset of its binary-place
// values. Each parcel then becomes one 0/1-knapsack item worth
// (parcel_size * denom) cents and costing parcel_size pieces.

#include <vector>
#include <cmath>
#include <climits>
using namespace std;

class Solution {
public:
    // inventory order: {$20,$10,$5,$1,$0.25,$0.10,$0.05,$0.01}
    int minPiecesLimited(double amount, vector<int> inventory) {
        long long cents = llround(amount * 100.0);
        if (cents == 0) return 0;   // zero change needs zero pieces, any drawer

        static const int denomsCents[8] = {2000, 1000, 500, 100, 25, 10, 5, 1};

        const int INF = INT_MAX / 2;
        vector<int> dp(static_cast<size_t>(cents) + 1, INF);
        dp[0] = 0;

        for (int i = 0; i < 8; ++i) {
            int remaining = inventory[i];
            int denom = denomsCents[i];

            // Binary-split this denomination's stock into O(log(inventory[i]))
            // parcels: 1, 2, 4, ... and a final remainder parcel.
            for (int parcel = 1; remaining > 0; parcel <<= 1) {
                int take = min(parcel, remaining);
                remaining -= take;

                long long value = static_cast<long long>(take) * denom;
                if (value > cents) continue;   // parcel alone overshoots target

                // Standard 0/1-knapsack update: iterate cents DOWNWARD so this
                // parcel is folded in at most once per pass (upward iteration
                // would let the same parcel be reused arbitrarily many times,
                // silently turning this into unbounded knapsack).
                for (long long j = cents; j >= value; --j) {
                    if (dp[j - value] + take < dp[j]) {
                        dp[j] = dp[j - value] + take;
                    }
                }
            }
        }

        return dp[cents] >= INF ? -1 : dp[cents];
    }
};

// Complexity: O(cents * sum_i log(inventory[i] + 1)) time, O(cents) space.
// (Naive one-coin-at-a-time bounded knapsack would be O(cents * sum(inventory)).)
