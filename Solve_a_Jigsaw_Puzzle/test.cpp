// Compile: g++ -std=c++17 -O2 -o test test.cpp && ./test
#include "solution.cpp"
#include <cassert>
#include <iostream>
#include <string>

// ── Match oracle ─────────────────────────────────────────────────────────────
// Two non-zero edges match iff they sum to 0.
// Zero is the "unused / border" placeholder; matchFn never pairs two zeros.
static bool matchFn(Edge a, Edge b) {
    return a != 0 && b != 0 && (a + b == 0);
}

// ── Verification helper ───────────────────────────────────────────────────────
// Walks every adjacent pair in the solved grid and asserts they match.
static bool verify(
    const std::vector<std::vector<PlacedPiece>>& grid,
    const std::vector<Piece>& pieces
) {
    const int R = static_cast<int>(grid.size());
    if (R == 0) return true;
    const int C = static_cast<int>(grid[0].size());

    for (int r = 0; r < R; ++r) {
        for (int c = 0; c < C; ++c) {
            const auto& [i, ri] = grid[r][c];
            if (i < 0) return false; // unset cell

            // Check right neighbor (horizontal interior edge)
            if (c + 1 < C) {
                const auto& [j, rj] = grid[r][c + 1];
                if (!matchFn(rotEdge(pieces[i], ri, 1),   // right of current
                             rotEdge(pieces[j], rj, 3)))  // left  of right neighbor
                    return false;
            }
            // Check bottom neighbor (vertical interior edge)
            if (r + 1 < R) {
                const auto& [j, rj] = grid[r + 1][c];
                if (!matchFn(rotEdge(pieces[i], ri, 2),   // bottom of current
                             rotEdge(pieces[j], rj, 0)))  // top    of bottom neighbor
                    return false;
            }
        }
    }
    return true;
}

static void check(const std::string& name, bool cond) {
    if (!cond) {
        std::cerr << "FAIL  " << name << "\n";
        std::exit(1);
    }
    std::cout << "PASS  " << name << "\n";
}

// ── Test 1: N = 0 ─────────────────────────────────────────────────────────────
// Empty input → empty grid returned immediately.
void test_empty() {
    auto grid = solve({}, matchFn);
    check("empty: grid is empty", grid.empty());
}

// ── Test 2: N = 1 ─────────────────────────────────────────────────────────────
// Trivial 1×1; no adjacency constraints. Any rotation is valid.
void test_single_piece() {
    std::vector<Piece> pieces = {{{7, 3, -5, 0}}};
    auto grid = solve(pieces, matchFn);
    check("single_piece: is 1×1",   grid.size() == 1 && grid[0].size() == 1);
    check("single_piece: idx == 0", grid[0][0].idx == 0);
}

// ── Test 3: N = 2, 1×2 strip ─────────────────────────────────────────────────
// Piece 0 must sit left of piece 1 (or vice versa via rotation) such that
// piece_left.right matches piece_right.left.
void test_1x2_strip() {
    std::vector<Piece> pieces = {
        {{0,  5, 0,  0}},  // piece 0: right edge = 5
        {{0,  0, 0, -5}},  // piece 1: left  edge = -5
    };
    auto grid = solve(pieces, matchFn);
    check("1x2: valid adjacency",  verify(grid, pieces));
    check("1x2: is 1×2 grid",     grid.size() == 1 && grid[0].size() == 2);
}

// ── Test 4: N = 4, 2×2 grid (input shuffled) ─────────────────────────────────
// Correct layout (using original piece names):
//   P0=[0,1,2,0]  P1=[0,0,3,-1]
//   P2=[-2,4,0,0] P3=[-3,0,0,-4]
//
// Edges that must match:
//   P0.right(1)  ↔ P1.left(-1)     P1.bottom(3) ↔ P3.top(-3)
//   P0.bottom(2) ↔ P2.top(-2)      P2.right(4)  ↔ P3.left(-4)
//
// Pieces are given to the solver in shuffled order [P3, P0, P2, P1].
void test_2x2_grid() {
    std::vector<Piece> pieces = {
        {{-3,  0,  0, -4}},  // idx 0  (was P3: bottom-right)
        {{ 0,  1,  2,  0}},  // idx 1  (was P0: top-left)
        {{-2,  4,  0,  0}},  // idx 2  (was P2: bottom-left)
        {{ 0,  0,  3, -1}},  // idx 3  (was P1: top-right)
    };
    auto grid = solve(pieces, matchFn);
    check("2x2: valid adjacency",  verify(grid, pieces));
    check("2x2: is 2×2 grid",     grid.size() == 2 && grid[0].size() == 2);
}

// ── Test 5: Rotation required ─────────────────────────────────────────────────
// Piece 0: edges = [0, 0, 0, 7]  → left=7 in original orientation.
//   At rot=2 (180°): rotEdge(p, 2, 1) = p.e[(1-2+4)%4] = p.e[3] = 7  → right=7.
// Piece 1: edges = [0, 0, 0, -7] → left=-7.
//   At rot=0: left=-7; needs a right=7 neighbor.
// The only valid connection in a 1×2 strip requires piece 0 at rot=2 on the left.
//
// Edge case: zero-valued edges are never checked (matchFn requires both non-zero),
//            so the only interior constraint is horizontal between the two pieces.
void test_rotation_required() {
    std::vector<Piece> pieces = {
        {{0, 0, 0,  7}},  // left=7; exposed as right only at rot=2
        {{0, 0, 0, -7}},  // left=-7; exposed as right only at rot=2
    };
    auto grid = solve(pieces, matchFn);
    check("rotation: valid adjacency",  verify(grid, pieces));
    check("rotation: is 1×2 grid",     grid.size() == 1 && grid[0].size() == 2);

    // Confirm the left piece in the solution uses a non-zero rotation.
    const PlacedPiece& left = grid[0][0];
    // rotEdge at right direction must be 7 or -7 (not 0) for the match to work.
    check("rotation: left piece exposes non-zero right",
          rotEdge(pieces[left.idx], left.rot, 1) != 0);
}

// ── Test 6: N = 4, 1×4 strip (2×2 impossible; solver must exhaust it first) ──
// Horizontal chain: A→B→C→D.
// Vertical edges are all zero — matchFn will reject them, so a 2×2 grid
// (which would require vertical matches) fails, and the solver falls back
// to the 1×4 strip shape.
void test_1x4_strip_after_2x2_fails() {
    std::vector<Piece> pieces = {
        {{ 0, 20,  0,  0}},  // A: right=20
        {{ 0, 21,  0, -20}}, // B: right=21, left=-20
        {{ 0, 22,  0, -21}}, // C: right=22, left=-21
        {{ 0,  0,  0, -22}}, // D: left=-22
    };
    auto grid = solve(pieces, matchFn);
    check("1x4: valid adjacency", verify(grid, pieces));
    const int total = static_cast<int>(grid.size() * grid[0].size());
    check("1x4: all 4 pieces placed", total == 4);
}

// ── Test 7: N = 6, 2×3 grid ───────────────────────────────────────────────────
// Correct layout:
//   A=[0,10,14,0]    B=[0,11,15,-10]   C=[0,0,16,-11]
//   D=[-14,12,0,0]   E=[-15,13,0,-12]  F=[-16,0,0,-13]
//
// Horizontal chains: A-B-C (top row) and D-E-F (bottom row).
// Vertical links:    A↓D (14/-14), B↓E (15/-15), C↓F (16/-16).
//
// Critically, horizontal chains A-B-C and D-E-F cannot be merged into a single
// 1×6 strip because no cross-group edge pair sums to zero while both being
// non-zero — the solver will exhaust (1×6) and (6×1) before finding (2×3) or (3×2).
// Input is given in order [F, A, D, C, B, E] to exercise the piece selection.
void test_2x3_grid() {
    std::vector<Piece> original = {
        {{ 0, 10, 14,   0}},  // A
        {{ 0, 11, 15, -10}},  // B
        {{ 0,  0, 16, -11}},  // C
        {{-14, 12,  0,   0}}, // D
        {{-15, 13,  0, -12}}, // E
        {{-16,  0,  0, -13}}, // F
    };
    std::vector<Piece> pieces = {
        original[5], original[0], original[3],  // F, A, D
        original[2], original[1], original[4],  // C, B, E
    };
    auto grid = solve(pieces, matchFn);
    check("2x3: valid adjacency", verify(grid, pieces));
    check("2x3: is 2×3 or 3×2",
          (grid.size() == 2 && grid[0].size() == 3) ||
          (grid.size() == 3 && grid[0].size() == 2));
}

// ── Test 8: All-same-edge piece (4-fold symmetric) ────────────────────────────
// Piece where every edge is the same non-zero value: all 4 rotations are
// equivalent. For a 1×2 strip, piece 0 (right=5) connects to piece 1 (left=-5).
// The symmetric piece (all edges = 5) can go in either slot — verifies the
// solver handles ambiguity without infinite loops or double-placement.
void test_symmetric_piece() {
    std::vector<Piece> pieces = {
        {{ 5,  5,  5,  5}},  // symmetric: all edges = 5
        {{ 0,  0,  0, -5}},  // left=-5, connects to any right=5
    };
    auto grid = solve(pieces, matchFn);
    check("symmetric: valid adjacency",  verify(grid, pieces));
    check("symmetric: is 1×2 grid",     grid.size() == 1 && grid[0].size() == 2);
}

// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "Running jigsaw solver tests...\n\n";

    test_empty();
    test_single_piece();
    test_1x2_strip();
    test_2x2_grid();
    test_rotation_required();
    test_1x4_strip_after_2x2_fails();
    test_2x3_grid();
    test_symmetric_piece();

    std::cout << "\nAll tests passed.\n";
    return 0;
}
