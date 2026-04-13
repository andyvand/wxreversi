/****************************************************************************/
/*                                                                          */
/*  reversi_engine.h                                                        */
/*                                                                          */
/*  Portable Reversi game engine, extracted from the original Windows       */
/*  Reversi sample (Chris Peters).  The engine contains no GUI code; all    */
/*  board state, move generation, scoring and alpha-beta search lives       */
/*  here and is callable from any C++ front-end (wxWidgets, Qt, SDL...).    */
/*                                                                          */
/****************************************************************************/

#ifndef REVERSI_ENGINE_H
#define REVERSI_ENGINE_H

#include <cstdint>

namespace reversi {

/* Board layout: the board is stored as a 10x10 byte array flattened to 100
 * bytes.  Only squares 11..88 (using pseudo-decimal indexing, x*10 + y + 11
 * with x,y in 0..7) are playable.  The rest are sentinel "edge" squares
 * which simplifies move-flipping loops (they can safely walk off the edge).
 */

enum Cell : std::uint8_t {
    EDGE     = 0,
    EMPTY    = 1,
    HUMAN    = 2,
    COMPUTER = 3
};

static const int BoardSize = 100;
static const int MaxDepth  = 6;    // maximum search ply
static const int Infin     = 32767;
static const int WinScore  =  32000;
static const int LossScore = -32000;
static const int PASS      = 20;   // sentinel "move" indicating a pass

/* The 61 playable squares in preferred move order (corners first, then
 * edges, then interior).  Terminated with 0.                              */
extern const int MoveOrder[61];

/* Eight compass directions used for flipping; terminated with 0.          */
extern const int Directions[9];

/* Convert (x,y) in [0,7] to the internal board index. */
inline int xyToIndex(int x, int y) { return x * 10 + y + 11; }
inline int indexToX(int idx)       { return (idx - 11) / 10; }
inline int indexToY(int idx)       { return (idx - 11) % 10; }

/*
 * Engine state.
 *
 * The engine keeps MaxDepth+2 board copies so the recursive alpha-beta
 * search can scratch them without heap allocation, just like the original.
 */
class Engine {
public:
    Engine();

    // Reset to the standard starting position (four centre pieces).
    void reset();

    // Direct access to the current (ply 0) board.
    std::uint8_t& at(int idx)             { return m_boards[0][idx]; }
    std::uint8_t  at(int idx) const       { return m_boards[0][idx]; }
    std::uint8_t  at(int x, int y) const  { return m_boards[0][xyToIndex(x, y)]; }

    // Search depth (1, 2, 4, or 6 in the original game).
    int  depth() const       { return m_depth; }
    void setDepth(int d)     { m_depth = d; }

    // Pure move legality test.  Does not mutate the board.
    static bool isLegal(const std::uint8_t* b, int move,
                        std::uint8_t friendly, std::uint8_t enemy);

    // Apply a move (flipping enemy pieces) to a given board.
    static void applyMove(std::uint8_t* b, int move,
                          std::uint8_t friendly, std::uint8_t enemy);

    // Count pieces of a given colour on the ply-0 board.
    int count(std::uint8_t who) const;

    // Is there at least one legal move for the given player?
    bool hasAnyLegalMove(std::uint8_t friendly, std::uint8_t enemy) const;

    /*
     * Run alpha-beta search and return the best reply to `lastMove` for the
     * side `enemy`, using `friendly` as the mover who just played lastMove.
     * The caller is responsible for applying `lastMove` to the board before
     * the call (matching the original code's convention).
     *
     * Returns the chosen move index (11..88) or PASS if no legal reply.
     */
    int searchBestReply(int lastMove,
                        std::uint8_t friendly,
                        std::uint8_t enemy);

    // Best-move hint for the given side (used by the "Hint" menu).
    int bestMoveHint(std::uint8_t friendly, std::uint8_t enemy);

private:
    int  minmax(int move, std::uint8_t friendly, std::uint8_t enemy,
                int ply, int vmin, int vmax);
    static int score(const std::uint8_t* b,
                     std::uint8_t friendly, std::uint8_t enemy);
    static int finalScore(const std::uint8_t* b,
                          std::uint8_t friendly, std::uint8_t enemy);

    std::uint8_t m_boards[MaxDepth + 2][BoardSize];
    int          m_bestMove[MaxDepth + 2];
    int          m_depth;
};

} // namespace reversi

#endif // REVERSI_ENGINE_H
