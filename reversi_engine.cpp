/****************************************************************************/
/*                                                                          */
/*  reversi_engine.cpp                                                      */
/*                                                                          */
/*  Portable Reversi engine.  The alpha-beta search, move generation and    */
/*  scoring functions are a direct, de-Win32-ified translation of the       */
/*  original rev.c written by Chris Peters.                                 */
/*                                                                          */
/****************************************************************************/

#include "reversi_engine.h"

namespace reversi {

const int MoveOrder[61] = {
    11, 18, 81, 88,  13, 31, 16, 61,
    38, 83, 68, 86,  14, 41, 15, 51,
    48, 84, 58, 85,  33, 36, 63, 66,
    34, 35, 43, 46,  53, 56, 64, 65,
    24, 25, 42, 47,  52, 57, 74, 75,
    23, 26, 32, 37,  62, 67, 73, 76,
    12, 17, 21, 28,  71, 78, 82, 87,
    22, 27, 72, 77,
    0
};

const int Directions[9] = { 9, 10, 11, 1, -1, -9, -10, -11, 0 };

Engine::Engine() : m_depth(1)
{
    reset();
}

void Engine::reset()
{
    for (int i = 0; i <= MaxDepth + 1; ++i)
        for (int j = 0; j < BoardSize; ++j)
            m_boards[i][j] = EDGE;

    for (int i = 0; i <= MaxDepth + 1; ++i) {
        for (int j = 11; j <= 81; j += 10)
            for (int k = j; k < j + 8; ++k)
                m_boards[i][k] = EMPTY;

        m_boards[i][45] = COMPUTER;
        m_boards[i][54] = COMPUTER;
        m_boards[i][44] = HUMAN;
        m_boards[i][55] = HUMAN;
    }

    for (int i = 0; i < MaxDepth + 2; ++i)
        m_bestMove[i] = PASS;
}

bool Engine::isLegal(const std::uint8_t* b, int move,
                     std::uint8_t friendly, std::uint8_t enemy)
{
    if (b[move] != EMPTY)
        return false;

    for (const int* p = Directions; *p != 0; ++p) {
        int d  = *p;
        int sq = move + d;
        if (b[sq] != enemy)
            continue;
        do {
            sq += d;
        } while (b[sq] == enemy);
        if (b[sq] == friendly)
            return true;
    }
    return false;
}

void Engine::applyMove(std::uint8_t* b, int move,
                       std::uint8_t friendly, std::uint8_t enemy)
{
    if (move == PASS)
        return;

    for (const int* p = Directions; *p != 0; ++p) {
        int d  = *p;
        int sq = move + d;
        if (b[sq] != enemy)
            continue;
        do {
            sq += d;
        } while (b[sq] == enemy);
        if (b[sq] == friendly) {
            while (b[sq -= d] == enemy)
                b[sq] = friendly;
        }
    }
    b[move] = friendly;
}

int Engine::count(std::uint8_t who) const
{
    int n = 0;
    for (int i = 11; i <= 88; ++i)
        if (m_boards[0][i] == who)
            ++n;
    return n;
}

bool Engine::hasAnyLegalMove(std::uint8_t friendly, std::uint8_t enemy) const
{
    for (const int* p = MoveOrder; *p != 0; ++p)
        if (isLegal(m_boards[0], *p, friendly, enemy))
            return true;
    return false;
}

int Engine::finalScore(const std::uint8_t* b,
                       std::uint8_t friendly, std::uint8_t enemy)
{
    int count = 0;
    for (int i = 11; i <= 88; ++i) {
        if (b[i] == friendly)      ++count;
        else if (b[i] == enemy)    --count;
    }
    if (count > 0)  return WinScore  + count;
    if (count == 0) return 0;
    return LossScore + count;
}

int Engine::score(const std::uint8_t* b,
                  std::uint8_t friendly, std::uint8_t enemy)
{
    /* Positional scoring tables, unchanged from the original engine.  Two
     * tables are used: `value` when all four corners are empty, `value2`
     * (with corner-adjacent squares re-weighted) once a corner has fallen.
     */
    static int value[79] = {
         99, -8,  8,  6,  6,  8, -8, 99,   0,
          0, -8,-24, -4, -3, -3, -4,-24, -8,   0,
          0,  8, -4,  7,  4,  4,  7, -4,  8,   0,
          0,  6, -3,  4,  0,  0,  4, -3,  6,   0,
          0,  6, -3,  4,  0,  0,  4, -3,  6,   0,
          0,  8, -4,  7,  4,  4,  7, -4,  8,   0,
          0, -8,-24, -4, -3, -3, -4,-24, -8,   0,
          0, 99, -8,  8,  6,  6,  8, -8, 99, Infin
    };
    static int value2[79] = {
         99, -8,  8,  6,  6,  8, -8, 99,   0,
          0, -8,-24,  0,  1,  1,  0,-24, -8,   0,
          0,  8,  0,  7,  4,  4,  7,  0,  8,   0,
          0,  6,  1,  4,  1,  1,  4,  1,  6,   0,
          0,  6,  1,  4,  1,  1,  4,  1,  6,   0,
          0,  8,  0,  7,  4,  4,  7,  0,  8,   0,
          0, -8,-24,  0,  1,  1,  1,-24, -8,   0,
          0, 99, -8,  8,  6,  6,  8, -8, 99, Infin
    };

    int b11 = b[11], b18 = b[18], b81 = b[81], b88 = b[88];
    int* pv;

    if (b11 != EMPTY || b18 != EMPTY || b81 != EMPTY || b88 != EMPTY) {
        pv = value2;
        // Adjust the "C-squares" / "X-squares" next to each corner.
        if (b11 == EMPTY) { value2[ 1]=-8; value2[10]=-8; value2[11]=-24; }
        else              { value2[ 1]=12; value2[10]=12; value2[11]=  8; }
        if (b18 == EMPTY) { value2[ 6]=-8; value2[17]=-8; value2[16]=-24; }
        else              { value2[ 6]=12; value2[17]=12; value2[16]=  8; }
        if (b81 == EMPTY) { value2[71]=-8; value2[60]=-8; value2[61]=-24; }
        else              { value2[71]=12; value2[60]=12; value2[61]=  8; }
        if (b88 == EMPTY) { value2[76]=-8; value2[67]=-8; value2[66]=-24; }
        else              { value2[76]=12; value2[67]=12; value2[66]=  8; }
    } else {
        pv = value;
    }

    const std::uint8_t* pb = &b[11];
    int fpoints = 0, epoints = 0, ecount = 0, v;
    while ((v = *pv++) != Infin) {
        std::uint8_t bv = *pb++;
        if (bv == friendly) {
            fpoints += v;
        } else if (bv == enemy) {
            epoints += v;
            ++ecount;
        }
    }
    if (!ecount)
        return WinScore;    // wiped out the opponent
    return fpoints - epoints;
}

int Engine::minmax(int move, std::uint8_t friendly, std::uint8_t enemy,
                   int ply, int vmin, int vmax)
{
    std::uint8_t* cur  = m_boards[ply + 1];
    const std::uint8_t* src = m_boards[ply];

    for (int i = 11; i <= 88; ++i)
        cur[i] = src[i];

    int* pBest = &m_bestMove[ply];

    if (move == PASS) {
        if (ply == m_depth) {
            for (const int* pM = MoveOrder; *pM; ++pM)
                if (isLegal(cur, *pM, enemy, friendly))
                    return score(cur, friendly, enemy);
            return finalScore(cur, friendly, enemy);
        }
    } else {
        // At ply 0 the caller has already *placed* `move` on m_boards[0]
        // (matching the original code's convention), but has not flipped
        // the captured enemy pieces.  Apply the flips now on the scratch
        // board so the search evaluates legal replies against the true
        // post-move position.
        applyMove(cur, move, friendly, enemy);
        if (ply == m_depth)
            return score(cur, friendly, enemy);
    }

    int curMove = PASS;
    *pBest = PASS;

    for (const int* pM = MoveOrder; *pM; ++pM) {
        int sq = *pM;
        if (isLegal(cur, sq, enemy, friendly)) {
            curMove = sq;
            int v = minmax(curMove, enemy, friendly, ply + 1, -vmax, -vmin);
            if (v > vmin) {
                vmin = v;
                *pBest = curMove;
                if (v >= vmax)
                    goto cutoff;      // alpha-beta cutoff
            }
        }
    }

    if (curMove == PASS) {
        if (move == PASS)
            return finalScore(cur, friendly, enemy);   // two passes -> game over
        int v = minmax(PASS, enemy, friendly, ply + 1, -vmax, -vmin);
        if (v > vmin)
            vmin = v;
    }

cutoff:
    return -vmin;
}

int Engine::searchBestReply(int lastMove,
                            std::uint8_t friendly,
                            std::uint8_t enemy)
{
    minmax(lastMove, friendly, enemy, 0, -Infin, Infin);
    return m_bestMove[0];
}

int Engine::bestMoveHint(std::uint8_t friendly, std::uint8_t enemy)
{
    // At depth 1 the original just picks the first legal move from the
    // priority list.  At deeper depths it runs the search.
    if (m_depth <= 1) {
        for (const int* p = MoveOrder; *p; ++p)
            if (isLegal(m_boards[0], *p, friendly, enemy))
                return *p;
        return PASS;
    }
    minmax(m_bestMove[0], enemy, friendly, 1, -Infin, Infin);
    return m_bestMove[1];
}

} // namespace reversi
