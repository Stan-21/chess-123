#include "Chess.h"
#include "Bit.h"
#include "BitHolder.h"
#include "Bitboard.h"
#include "ChessSquare.h"
#include "Game.h"
#include "MagicBitboards.h"
#include "PieceSquare.h"
#include <cctype>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <cmath>
#include <vector>

Chess::Chess()
{
    _grid = new Grid(8, 8);
    for (int i = 0; i < 64; i++) {
        _knightBitboards[i] = generateKnightMoveBitBoard(i); // Figure out all possible ways a knight can move
        _kingBitboards[i] = generateKingMoveBitBoard(i);
    }
    initMagicBitboards(); // Generates the bishop, rook, and queen moves

    for (int i = 0; i < 128; i++) {_bitboardLookup[i] = 0; }
    _bitboardLookup['P'] = WHITE_PAWNS;
    _bitboardLookup['N'] = WHITE_KNIGHTS;
    _bitboardLookup['B'] = WHITE_BISHOPS;
    _bitboardLookup['R'] = WHITE_ROOKS;
    _bitboardLookup['Q'] = WHITE_QUEENS;
    _bitboardLookup['K'] = WHITE_KING;
    _bitboardLookup['p'] = BLACK_PAWNS;
    _bitboardLookup['n'] = BLACK_KNIGHTS;
    _bitboardLookup['b'] = BLACK_BISHOPS;
    _bitboardLookup['r'] = BLACK_ROOKS;
    _bitboardLookup['q'] = BLACK_QUEENS;
    _bitboardLookup['k'] = BLACK_KING;
    _bitboardLookup['0'] = EMPTY_SQUARES;

}

Chess::~Chess()
{
    cleanupMagicBitboards();
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

    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
    }

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    //FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    //FENtoBoard("6k1/5ppp/8/8/8/8/5PPP/5RK1 w - - 0 1");
    //FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/8/RNBQKBNR w KQkq - 0 1"); // For testing rooks / bishops / queens

    _currentPlayer = WHITE;
    _moves = generateAllMoves(stateString(), _currentPlayer);
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
    _moves = generateAllMoves(stateString(), _currentPlayer);;
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
    if (_moves.size() == 0) {
        std::cout << "There is a winner" << std::endl;
        return getPlayerAt(getCurrentPlayer()->playerNumber() == 0 ? 1 : 0);
    }
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
            moves.emplace_back(from, to, King);
        });
    });
}

void Chess::generatePawnMoves(std::vector<BitMove>& moves, BitboardElement pawnBoard, uint64_t occupancy, uint64_t opp_occupancy) {
    // Pawns can move 'forward' one space (white = 1, black = -1)
    // Pawns can move 'forward' two spaces IF: white and rank 1 OR black and rank 6
    int direction = _currentPlayer == WHITE ? 8 : -8;
    int doubleRank = _currentPlayer == WHITE ? 1 : 6;

    pawnBoard.forEachBit([&](int from) {
        int rank = from / 8;
        int to = from + direction;
        if (to >= 0 && to < 64) {
            if (occupancy & (1ULL << to)) {
                moves.emplace_back(from, to, Pawn);
                if (rank == doubleRank) {
                    int two = from + (direction * 2);
                    if (occupancy & (1ULL << two)) {
                        moves.emplace_back(from, two, Pawn);
                    }
                }
            }
        }
    });

    pawnBoard.forEachBit([&](int from) {
        int file = from % 8;
        if (file != 0) {
            int to = from + direction - 1;

            if (to >= 0 && to < 64) {
                if (opp_occupancy & (1ULL << to)) {
                    moves.emplace_back(from, to, Pawn);
                }
            }
        }
        if (file != 7) {
            int to = from + direction + 1;

            if (to >= 0 && to < 64) {
                if (opp_occupancy & (1ULL << to)) {
                    moves.emplace_back(from, to, Pawn);
                }
            }
        }
    });
}

void Chess::generateBishopMoves(std::vector<BitMove>& moves, BitboardElement bishopBoard, uint64_t occupancy, uint64_t self_occupancy) {
    bishopBoard.forEachBit([&](int from) {
        BitboardElement canMoveTo(getBishopAttacks(from, occupancy) & ~self_occupancy);
        canMoveTo.forEachBit([from, &moves](int to) {
            moves.emplace_back(from, to, Bishop);
        });
    });
}

void Chess::generateRookMoves(std::vector<BitMove>& moves, BitboardElement rookBoard, uint64_t occupancy, uint64_t self_occupancy) {
    rookBoard.forEachBit([&](int from) {
        BitboardElement canMoveTo(getRookAttacks(from, occupancy) & ~self_occupancy);
        canMoveTo.forEachBit([from, &moves](int to) {
            moves.emplace_back(from, to, Rook);
        });
    });
}

void Chess::generateQueenMoves(std::vector<BitMove>& moves, BitboardElement queenBoard, uint64_t occupancy, uint64_t self_occupancy) {
    queenBoard.forEachBit([&](int from) {
        BitboardElement canMoveTo(getQueenAttacks(from, occupancy) & ~self_occupancy);
        canMoveTo.forEachBit([from, &moves](int to) {
            moves.emplace_back(from, to, Queen);
        });
    });
}

std::vector<BitMove> Chess::generateAllMoves(std::string state, const int currentPlayer) {
    std::vector<BitMove> moves;
    moves.reserve(32);

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

    int bitIndex = currentPlayer == WHITE ? WHITE_PAWNS : BLACK_PAWNS;
    int oppBitIndex = currentPlayer == WHITE ? BLACK_PAWNS : WHITE_PAWNS; // not being used rn
    int selfOccupancyIndex = currentPlayer == WHITE ? WHITE_ALL_PIECES : BLACK_ALL_PIECES;
    int oppOccupancyIndex = currentPlayer == WHITE ? BLACK_ALL_PIECES : WHITE_ALL_PIECES;

    generateKnightMoves(moves, _bitboards[WHITE_KNIGHTS + bitIndex], 
        ~_bitboards[selfOccupancyIndex].getData());
    generateKingMoves(moves, _bitboards[WHITE_KING + bitIndex],
        ~_bitboards[selfOccupancyIndex].getData());
    generatePawnMoves(moves, _bitboards[WHITE_PAWNS + bitIndex],
    ~_bitboards[OCCUPANCY].getData(), _bitboards[oppOccupancyIndex].getData());
    
    generateBishopMoves(moves, _bitboards[WHITE_BISHOPS + bitIndex],
    _bitboards[OCCUPANCY].getData(), _bitboards[selfOccupancyIndex].getData());
    generateRookMoves(moves, _bitboards[WHITE_ROOKS + bitIndex],
    _bitboards[OCCUPANCY].getData(), _bitboards[selfOccupancyIndex].getData());
    generateQueenMoves(moves, _bitboards[WHITE_QUEENS + bitIndex],
    _bitboards[OCCUPANCY].getData(), _bitboards[selfOccupancyIndex].getData());
    removeIllegalMoves(moves, state, currentPlayer);
    return moves;
}

void Chess::removeIllegalMoves(std::vector<BitMove>& moves, std::string state, const int currentPlayer) {
    constexpr uint64_t NotAFile = 0xFEFEFEFEFEFEFEFEULL;
    constexpr uint64_t NotHFile = 0x7F7F7F7F7F7F7F7FULL;

    auto isKingInCheck = [&]() -> bool {
        uint64_t bitboards[e_numBitBoards] = {0};
        int kingSquare = -1;

        for (int sq = 0; sq < 64; sq++) {
            int bitIndex = _bitboardLookup[state[sq]];
            bitboards[bitIndex] |= (1ULL << sq);
        }

        bitboards[WHITE_ALL_PIECES] = bitboards[WHITE_PAWNS]
        | bitboards[WHITE_KNIGHTS]
        | bitboards[WHITE_BISHOPS]
        | bitboards[WHITE_ROOKS]
        | bitboards[WHITE_QUEENS]
        | bitboards[WHITE_KING];

        bitboards[BLACK_ALL_PIECES] = bitboards[BLACK_PAWNS]
        | bitboards[BLACK_KNIGHTS]
        | bitboards[BLACK_BISHOPS]
        | bitboards[BLACK_ROOKS]
        | bitboards[BLACK_QUEENS]
        | bitboards[BLACK_KING];


        uint64_t occupancy = bitboards[WHITE_ALL_PIECES] | bitboards[BLACK_ALL_PIECES];

        unsigned long index;
        uint64_t bb = bitboards[currentPlayer == WHITE ? WHITE_KING : BLACK_KING];
        #if defined(_MSC_VER) && !defined(__clang__)
                _BitScanForward64(&index, bb);
        #else
                index = __builtin_ffsll(bb) - 1; // Returns index of first bit with a value (ie 1) (right to left) and then subtracts 1
        #endif
        kingSquare = bitboards[currentPlayer == WHITE ? WHITE_KING : BLACK_KING] ? index : -1;

        if (kingSquare == -1) return true;

        int enemyBitIndexBase = (currentPlayer == WHITE) ? BLACK_PAWNS : WHITE_PAWNS;

        uint64_t kingBit = (1ULL << kingSquare);
        uint64_t enemyPawns = bitboards[enemyBitIndexBase + WHITE_PAWNS];
        uint64_t enemyKnights = bitboards[enemyBitIndexBase + WHITE_KNIGHTS];
        uint64_t enemyBishQueens = bitboards[enemyBitIndexBase + WHITE_BISHOPS] | bitboards[enemyBitIndexBase + WHITE_QUEENS];
        uint64_t enemyRookQueens = bitboards[enemyBitIndexBase + WHITE_ROOKS] | bitboards[enemyBitIndexBase + WHITE_QUEENS];
        uint64_t enemyKing = bitboards[enemyBitIndexBase + WHITE_KING];

        if (getRookAttacks(kingSquare, occupancy) & enemyRookQueens) return true;
        if (getBishopAttacks(kingSquare, occupancy) & enemyBishQueens) return true;
        if (KnightAttacks[kingSquare] & enemyKnights) return true;
        if (KingAttacks[kingSquare] & enemyKing) return true;
        uint64_t pawnAttacks;
        if (currentPlayer == WHITE) {
            pawnAttacks = ((kingBit & NotAFile) << 7) | ((kingBit & NotHFile) << 9);
        } else {
            pawnAttacks = ((kingBit & NotHFile) >> 7) | ((kingBit & NotAFile) >> 9);
        }
        if (pawnAttacks & enemyPawns) return true;

        return false;
    };

    auto it = moves.begin();
    while (it != moves.end()) {
        int srcSquare = it->from;
        int dstSquare = it->to;
        char saveMove = state[dstSquare];
        state[dstSquare] = state[srcSquare];
        state[srcSquare] = '0';
        bool illegal = isKingInCheck();
        state[srcSquare] = state[dstSquare];
        state[dstSquare] = saveMove;

        if (illegal) {
            it = moves.erase(it);
        } else {
            ++it;
        }
    }
}

void Chess::updateAI() {
    if (checkForWinner()) {
        endTurn();
        return;
    }
    std::cout << "updating AI" << std::endl;

    char baseState[65];
    const int myInfinity = 999999999;
    int bestMoveScore = -myInfinity; // Min value
    BitMove bestMove;
    std::string copyState = stateString();

    const auto start = std::chrono::high_resolution_clock::now();

    for (auto move : _moves) {
        strcpy(&baseState[0], copyState.c_str());
        int srcSquare = move.from;
        int dstSquare = move.to;
        baseState[dstSquare] = baseState[srcSquare];
        baseState[srcSquare] = '0';
        _countState = 0;
        int bestValue = -negaMax(baseState, 4, -myInfinity, myInfinity, -_currentPlayer);
        if (bestValue > bestMoveScore) {
            bestMoveScore = bestValue;
            bestMove = move;
        }
    }

    if (_moves.size() == 1) {
        bestMove = _moves[0];
        bestMoveScore += 1;
    }

    std::cout << "searched " << _countState << " nodes" << std::endl;

    if (bestMoveScore != -myInfinity) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        double nps = duration.count() > 0 ? (_countState * 1000.0 / duration.count()) : _countState;

        std::cout << "(" << duration.count() <<"ms, ";
        std::cout << (uint64_t)nps << " nps)\n";

        int srcSquare = bestMove.from;
        int dstSquare = bestMove.to;
        BitHolder& src = getHolderAt(srcSquare & 7, srcSquare / 8);
        BitHolder& dst = getHolderAt(dstSquare & 7, dstSquare / 8);
        Bit* bit = src.bit();
        dst.dropBitAtPoint(bit, ImVec2(0, 0));
        src.setBit(nullptr);
        bitMovedFromTo(*bit, src, dst);
    }
}

int Chess::negaMax(char* state, int depth, int alpha, int beta, int playerColor) {
    int score = Evaluate(state);
    _countState += 1;
    
    if (depth == 0) { return score * playerColor; }

    int bestMoveScore = -999999999; // Min value
    auto negaMoves = generateAllMoves(state, playerColor);

    if (negaMoves.empty()) {
        return -100000 + depth * playerColor;
    }

    for (auto move : negaMoves) {
        int srcSquare = move.from;
        int dstSquare = move.to;
        char saveMove = state[dstSquare];
        state[dstSquare] = state[srcSquare];
        state[srcSquare] = '0';
        bestMoveScore = std::max(bestMoveScore, -negaMax(state, depth - 1, -beta, -alpha, -playerColor));
        state[srcSquare] = state[dstSquare];
        state[dstSquare] = saveMove;

        alpha = std::max(alpha, bestMoveScore);
        if (alpha >= beta) {
            break;
        }
    }

    return bestMoveScore;
}

int Chess::Evaluate(const std::string& state) {
    static int coolValueArray[256];
    static int pieceSquareTable[256];
    static bool initedCool = false;
    if (!initedCool) {
        initedCool = true;
        coolValueArray['0'] = 0;
        coolValueArray['P'] = 100;
        coolValueArray['R'] = 500;
        coolValueArray['N'] = 300;
        coolValueArray['B'] = 400;
        coolValueArray['Q'] = 900;
        coolValueArray['K'] = 2000;
        coolValueArray['p'] = -100;
        coolValueArray['r'] = -500;
        coolValueArray['n'] = -300;
        coolValueArray['b'] = -400;
        coolValueArray['q'] = -900;
        coolValueArray['k'] = -2000;
    }

    int value = 0;
    int count = 0;

    for (char ch : state) {
        value += coolValueArray[ch];
        switch (ch) {
            case 'P':
                value += pawnTableW[count];
                break;
            case 'p':
                value += pawnTableB[count];
                break;
            case 'N':
                value += knightTableW[count];
                break;
            case 'n':
                value += knightTableB[count];
                break;
            case 'B':
                value += bishopTableW[count];
                break;
            case 'b':
                value += bishopTableB[count];
                break;
            case 'R':
                value += rookTableW[count];
                break;
            case 'r':
                value += rookTableB[count];
                break;

            case 'Q':
                value += queenTableW[count];
                break;
            case 'q':
                value += queenTableB[count];
                break;

            case 'K':
                value += kingTableW[count];
                break;
            case 'k':
                value += kingTableB[count];
                break;

            default:
                break;
        }
        count += 1;
    }

    return value;
}