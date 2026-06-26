#ifndef JIGSAW_SOLUTION_CPP
#define JIGSAW_SOLUTION_CPP

#include <vector>
#include <functional>
#include <algorithm>

using Edge   = int;
using MatchFn = std::function<bool(Edge, Edge)>;

struct Piece {
    Edge e[4]; // indices: 0=top, 1=right, 2=bottom, 3=left
};

struct PlacedPiece {
    int idx; // index into original pieces array (-1 = unset)
    int rot; // clockwise rotations applied: 0, 1, 2, or 3
};

// Edge of piece p after 'rot' clockwise 90° rotations, in direction 'dir'.
// Derivation: after r CW rotations, the edge at slot d came from original slot (d - r + 4) % 4.
inline Edge rotEdge(const Piece& p, int rot, int dir) {
    return p.e[(dir - rot + 4) % 4];
}

// Returns a grid[R][C] of PlacedPiece on success, or an empty vector if the
// input is empty. Tries all factor pairs (R, C) of N via backtracking with
// O(1) constraint checks from precomputed match tables.
//
// Complexity:
//   Precomputation : O(16N^2) match() calls, O(N^2) space
//   Backtracking   : O(N) stack depth; branching depends on edge selectivity
//   Worst case     : O(N^2) precompute + O((4k)^N) backtrack  (k = avg compatible edges)
std::vector<std::vector<PlacedPiece>> solve(
    const std::vector<Piece>& pieces,
    MatchFn match
) {
    const int N = static_cast<int>(pieces.size());
    if (N == 0) return {};

    // Encode (piece_idx, rotation) as a single integer: piece_idx * 4 + rotation.
    // Range: 0 .. 4N-1. This becomes the row/column index into the match tables.
    const int NR = N * 4;

    // preH[a][b]: right edge of oriented piece 'a' matches left edge of oriented piece 'b'.
    // preV[a][b]: bottom edge of oriented piece 'a' matches top edge of oriented piece 'b'.
    // We skip i == j because each piece is used exactly once — it can never be its own neighbor.
    std::vector<std::vector<bool>> preH(NR, std::vector<bool>(NR, false));
    std::vector<std::vector<bool>> preV(NR, std::vector<bool>(NR, false));

    for (int i = 0; i < N; ++i)
        for (int ri = 0; ri < 4; ++ri)
            for (int j = 0; j < N; ++j) {
                if (i == j) continue;
                for (int rj = 0; rj < 4; ++rj) {
                    preH[i*4+ri][j*4+rj] = match(rotEdge(pieces[i], ri, 1),   // right of i
                                                   rotEdge(pieces[j], rj, 3)); // left  of j
                    preV[i*4+ri][j*4+rj] = match(rotEdge(pieces[i], ri, 2),   // bottom of i
                                                   rotEdge(pieces[j], rj, 0)); // top    of j
                }
            }

    std::vector<bool>        used(N, false);
    std::vector<PlacedPiece> flat(N, {-1, 0}); // row-major flat grid

    // Fills cells 'pos'..'N-1' in reading order for a grid of shape (R × C).
    // Returns true and leaves 'flat' fully populated on success.
    std::function<bool(int, int, int)> bt = [&](int pos, int R, int C) -> bool {
        if (pos == N) return true;

        const int r = pos / C;
        const int c = pos % C;

        // -1 means "no neighbor" (border); otherwise it is the oriented key of the
        // already-placed neighbor, used for O(1) table lookup.
        const int topKey  = (r > 0) ? (flat[pos - C].idx * 4 + flat[pos - C].rot) : -1;
        const int leftKey = (c > 0) ? (flat[pos - 1].idx * 4 + flat[pos - 1].rot) : -1;

        for (int i = 0; i < N; ++i) {
            if (used[i]) continue;
            for (int rot = 0; rot < 4; ++rot) {
                const int key = i * 4 + rot;
                // Both checks are O(1) array lookups — no match() call at this point.
                if (topKey  >= 0 && !preV[topKey][key])  continue;
                if (leftKey >= 0 && !preH[leftKey][key]) continue;

                flat[pos] = {i, rot};
                used[i]   = true;
                if (bt(pos + 1, R, C)) return true;
                used[i]   = false; // undo — restore piece to the pool
            }
        }
        return false;
    };

    // Enumerate factor pairs (R, C) with R <= C to avoid trying the same square shape twice.
    // Using long long to guard against r*r overflow for large N.
    for (int r = 1; static_cast<long long>(r) * r <= static_cast<long long>(N); ++r) {
        if (N % r != 0) continue;
        const int c = N / r;

        // Try (R=r, C=c)
        std::fill(used.begin(), used.end(), false);
        if (bt(0, r, c)) {
            std::vector<std::vector<PlacedPiece>> grid(r, std::vector<PlacedPiece>(c));
            for (int k = 0; k < N; ++k) grid[k / c][k % c] = flat[k];
            return grid;
        }

        // Try transposed shape (R=c, C=r) — only if non-square
        if (r != c) {
            std::fill(used.begin(), used.end(), false);
            if (bt(0, c, r)) {
                std::vector<std::vector<PlacedPiece>> grid(c, std::vector<PlacedPiece>(r));
                for (int k = 0; k < N; ++k) grid[k / r][k % r] = flat[k];
                return grid;
            }
        }
    }

    return {}; // guaranteed not reached when input is well-formed
}

#endif // JIGSAW_SOLUTION_CPP
