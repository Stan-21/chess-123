#pragma once

#include "BitHolder.h"
#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"
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

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    std::vector<BitMove> generateAllMoves();
    BitboardElement generateKnightMoveBitBoard(int square); // Given a certain square, what are the 'L' positions that the knight can move to
    BitboardElement generateKingMoveBitBoard(int square);
    void generateKnightMoves(std::vector<BitMove>& moves, BitboardElement knightBoard, uint64_t occupancy);
    void generateKingMoves(std::vector<BitMove>& moves, BitboardElement kingBoard, uint64_t occupancy);

    int _currentPlayer;
    Grid* _grid;
    BitboardElement _knightBitboards[64];
    BitboardElement _kingBitboards[64];
    std::vector<BitMove>        _moves;
    BitboardElement _bitboards[e_numBitBoards]; // Array of bitboards
    int _bitboardLookup[128]; // Not sure on what this does YET
};