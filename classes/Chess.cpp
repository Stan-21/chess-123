#include "Chess.h"
#include "Bitboard.h"
#include "ChessSquare.h"
#include <cctype>
#include <cstdint>
#include <limits>
#include <cmath>

Chess::Chess()
{
    _grid = new Grid(8, 8);
    for (int i = 0; i < 64; i++) {
        _knightBitboards[i] = generateKnightMoveBitBoard(i); // Figure out all possible ways a knight can move
        _kingBitboards[i] = generateKingMoveBitBoard(i);
    }

    for (int i = 0; i < 128; i++) {_bitboardLookup[i] = 0; }
    _bitboardLookup['W'] = WHITE_PAWNS;
    _bitboardLookup['N'] = WHITE_KNIGHTS;
    _bitboardLookup['B'] = WHITE_BISHOPS;
    _bitboardLookup['R'] = WHITE_ROOKS;
    _bitboardLookup['Q'] = WHITE_QUEENS;
    _bitboardLookup['K'] = WHITE_KING;
    _bitboardLookup['w'] = BLACK_PAWNS;
    _bitboardLookup['n'] = BLACK_KNIGHTS;
    _bitboardLookup['b'] = BLACK_BISHOPS;
    _bitboardLookup['r'] = BLACK_ROOKS;
    _bitboardLookup['q'] = BLACK_QUEENS;
    _bitboardLookup['k'] = BLACK_KING;
    _bitboardLookup['0'] = EMPTY_SQUARES;

}

Chess::~Chess()
{
    delete _grid;
}

char Chess:: pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    //FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    _currentPlayer = WHITE;
    _moves = generateAllMoves();
    startGame();
}

void Chess::FENtoBoard(const std::string& fen) {
    // convert a FEN string to a board
    // FEN is a space delimited string with 6 fields
    // 1: piece placement (from white's perspective)
    // NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
    // ARE BELOW
    // 2: active color (W or B)
    // 3: castling availability (KQkq or -)
    // 4: en passant target square (in algebraic notation, or -)
    // 5: halfmove clock (number of halfmoves since the last capture or pawn advance)

    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->setBit(nullptr);
    });

    int row = 7; // row 7 is the top of the board 
    int col = 0;

    // Kinda did copy from the lecture, but I do understand how it works
    for (char ch : fen) {
        // Ignoring the last bit of extended fen string for now
        if (ch == ' ') {
            break;
        }
        if (ch == '/') {
            row--;
            col = 0;
        } else if (std::isdigit(ch)) {
            col += ch - '0';
        } else {
            ChessPiece piece = Pawn;
            switch (std::toupper(ch)) {
                case 'R':
                    piece = Rook;
                    break;
                case 'N':
                    piece = Knight;
                    break;
                case 'B':
                    piece = Bishop;
                    break;
                case 'Q':
                    piece = Queen;
                    break;
                case 'K':
                    piece = King;
                    break;
            }
            // If it is uppercase, make it a white piece
            Bit* bit = PieceForPlayer(std::isupper(ch) ? 0 : 1, piece);
            ChessSquare* square = _grid->getSquare(col, row);
            bit->setPosition(square->getPosition());
            bit->setParent(square);
            bit->setGameTag(std::isupper(ch) ? piece : (piece + 128));
            square->setBit(bit);
            col++;
        }
    }
}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if (pieceColor == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    ChessSquare* squareSrc = (ChessSquare *)&src;
    ChessSquare* squareDst = (ChessSquare *)&dst;

    int squareIndexSrc = squareSrc->getSquareIndex();
    int squareIndexDst = squareDst->getSquareIndex();
    for (auto move: _moves) {
        if (move.from == squareIndexSrc && move.to == squareIndexDst) {
            return true;
        }
    }
    return false;
}

void Chess::clearBoardHighlights() {
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->setHighlighted(false);
    });
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
    _currentPlayer = (_currentPlayer == WHITE ? BLACK : WHITE);
    _moves = generateAllMoves();
    clearBoardHighlights();
    endTurn();
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}

// Maybe can move Knight and King into one function
BitboardElement Chess::generateKnightMoveBitBoard(int square) {
    std::pair<int, int> possiblePositions[] = {
        {2, 1}, {-2, 1}, {2, -1}, {-2, -1},
        {1, 2}, {-1, 2}, {1, -2}, {-1, -2}
    };

    int file = square % 8;
    int rank = square / 8;

    uint64_t data = 0;
    for (int i = 0; i < 8; i++) {
        int newFile = file + possiblePositions[i].first;
        int newRank = rank + possiblePositions[i].second;
        if (newFile >= 0 && newFile < 8 && newRank >= 0 && newRank < 8) {
            int pos = newFile + (newRank * 8);
            data |= 1ULL << pos;
        }
    }

    return BitboardElement(data);
}

BitboardElement Chess::generateKingMoveBitBoard(int square) {
    std::pair<int, int> possiblePositions[] = {
        {1, 0}, {-1, 0},
        {0, 1}, {0, -1},
        {1, -1}, {-1, 1},
        {1, 1}, {-1, -1}
    };

    int file = square % 8;
    int rank = square / 8;

    uint64_t data = 0;
    for (int i = 0; i < 8; i++) {
        int newFile = file + possiblePositions[i].first;
        int newRank = rank + possiblePositions[i].second;
        if (newFile >= 0 && newFile < 8 && newRank >= 0 && newRank < 8) {
            int pos = newFile + (newRank * 8);
            data |= 1ULL << pos;
        }
    }
    return BitboardElement(data);
}

void Chess::generateKnightMoves(std::vector<BitMove>& moves, BitboardElement knightBoard, uint64_t occupancy) {
    // PSUEDO-CODE:
    // Loop through the knight bit board until we find a knight
    // Once we find a knight, keep track of that position
    // Check that position in [x] index of _knightBitboards
    // (not 100% sure on how to kill enemies right now)
    // Compare knightBoard and occupancy board, if occupied that is not a valid move
    // Append the remaining valid moves to moves

    knightBoard.forEachBit([&](int from) {
        // index is the position of a knight
        BitboardElement canMoveTo(_knightBitboards[from].getData() & occupancy);
        canMoveTo.forEachBit([from, &moves](int to) {
            moves.emplace_back(from, to, Knight);
        });
    });
}

void Chess::generateKingMoves(std::vector<BitMove>& moves, BitboardElement kingBoard, uint64_t occupancy) {
    kingBoard.forEachBit([&](int from) {
        BitboardElement canMoveTo(_kingBitboards[from].getData() & occupancy);
        canMoveTo.forEachBit([from, &moves](int to) {
            moves.emplace_back(from, to, Knight);
        });
    });
}


std::vector<BitMove> Chess::generateAllMoves() {
    std::vector<BitMove> moves;
    moves.reserve(32);
    std::string state = stateString();

    // Clears the bitboard
    for (int i = 0; i < e_numBitBoards; i++) {
        _bitboards[i] = 0;
    }


    for (int i = 0; i < 64; i++) {
        int bitIndex = _bitboardLookup[state[i]];
        _bitboards[bitIndex] |= 1ULL << i; // Turns on the bitboard at position i
        if (state[i] != '0') { // If there is actually a piece, also set the occupancy board / all pieces
            _bitboards[OCCUPANCY] |= 1ULL << i;
            _bitboards[isupper(state[i]) ? WHITE_ALL_PIECES : BLACK_ALL_PIECES] |= 1ULL << i;
        }
    }

    int bitIndex = _currentPlayer == WHITE ? WHITE_PAWNS : BLACK_PAWNS;
    int oppBitIndex = _currentPlayer == WHITE ? BLACK_PAWNS : WHITE_PAWNS; // not being used rn
    int occupancyIndex = _currentPlayer == WHITE ? WHITE_ALL_PIECES : BLACK_ALL_PIECES;

    generateKnightMoves(moves, _bitboards[WHITE_KNIGHTS + bitIndex], 
        ~_bitboards[occupancyIndex].getData());
    generateKingMoves(moves, _bitboards[WHITE_KING + bitIndex],
        ~_bitboards[occupancyIndex].getData());
    return moves;
}