#pragma once

#include "Bit.h"
#include "BitHolder.h"
#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"
#include "MagicBitboards.h"
#include <cassert>
#include <cstdint>

constexpr int pieceSize = 80;
constexpr int WHITE = +1;
constexpr int BLACK = -1;

enum AllBitBoards
{
    WHITE_PAWNS,
    WHITE_KNIGHTS,
    WHITE_BISHOPS,
    WHITE_ROOKS,
    WHITE_QUEENS,
    WHITE_KING,
    BLACK_PAWNS,
    BLACK_KNIGHTS,
    BLACK_BISHOPS,
    BLACK_ROOKS,
    BLACK_QUEENS,
    BLACK_KING,
    WHITE_ALL_PIECES,
    BLACK_ALL_PIECES,
    OCCUPANCY,
    EMPTY_SQUARES,
    e_numBitBoards
};

constexpr int MAX_DEPTH = 25;

struct GameState {
    char state[64];

    GameState() { stackLevel = 0; }
    static int stackLevel;
    static int srcSquares[MAX_DEPTH];
    static int dstSquares[MAX_DEPTH];
    static char saveMove[MAX_DEPTH];

    void pushMove(BitMove& move) {
        srcSquares[stackLevel] = move.from;
        dstSquares[stackLevel] = move.to;

        char saveMove = state[dstSquares[stackLevel]];
        state[dstSquares[stackLevel]] = state[srcSquares[stackLevel]];
        state[srcSquares[stackLevel]] = '0';
        stackLevel++;
        assert(stackLevel < MAX_DEPTH);
    }

    void popMove() {
        stackLevel--;
        assert(stackLevel >= 0);
        state[srcSquares[stackLevel]] = state[dstSquares[stackLevel]];
        state[dstSquares[stackLevel]] = saveMove[stackLevel];
    }
};

class Chess : public Game
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;

    void clearBoardHighlights() override;
    void bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;

    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;

    Grid* getGrid() override { return _grid; }

    bool gameHasAI() override { return true; }
    void updateAI() override; // Maybe this is supposed to be in public?
private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    std::vector<BitMove> generateAllMoves(std::string state, const int currentPlayer);
    BitboardElement generateKnightMoveBitBoard(int square); // Given a certain square, what are the 'L' positions that the knight can move to
    BitboardElement generateKingMoveBitBoard(int square);
    void generateKnightMoves(std::vector<BitMove>& moves, BitboardElement knightBoard, uint64_t occupancy);
    void generateKingMoves(std::vector<BitMove>& moves, BitboardElement kingBoard, uint64_t occupancy);
    void generatePawnMoves(std::vector<BitMove>& moves, BitboardElement pawnBoard, uint64_t occupancy, uint64_t opp_occupancy);

    void generateBishopMoves(std::vector<BitMove>& moves, BitboardElement bishopBoard, uint64_t occupancy, uint64_t opp_occupancy);
    void generateRookMoves(std::vector<BitMove>& moves, BitboardElement rookBoard, uint64_t occupancy, uint64_t opp_occupancy);
    void generateQueenMoves(std::vector<BitMove>& moves, BitboardElement queenBoard, uint64_t occupancy, uint64_t opp_occupancy);

    int negaMax(char* state, int depth, int alpha, int beta, int playerColor);
    int Evaluate(const std::string& state);

    int _currentPlayer;
    int _countState;
    Grid* _grid;
    BitboardElement _knightBitboards[64];
    BitboardElement _kingBitboards[64];
    std::vector<BitMove>        _moves;
    BitboardElement _bitboards[e_numBitBoards]; // Array of bitboards
    int _bitboardLookup[128]; // Not sure on what this does YET
};