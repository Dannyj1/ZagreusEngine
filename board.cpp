//
// Created by Danny on 22-3-2022.
//

#include <stdexcept>
#include <iostream>
#include <utility>
#include <random>

#include "board.h"
#include "piece.h"
#include "tt.h"

namespace Chess {
    Board::Board() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;

        for (uint64_t &zobristConstant : zobristConstants) {
            zobristConstant = dis(gen);
        }

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                Tile tile = Tile{x, y};

                board[x][y] = tile;
            }
        }

        setPieceAtPosition(4, 0, new King(PieceColor::BLACK, 4, 0, BLACK_KING_ID));
        setPieceAtPosition(4, 7, new King(PieceColor::WHITE, 4, 7, WHITE_KING_ID));
        setPieceAtPosition(0, 0, new Rook(PieceColor::BLACK, 0, 0, BLACK_A_ROOK_ID));
        setPieceAtPosition(0, 7, new Rook(PieceColor::WHITE, 0, 7, WHITE_A_ROOK_ID));
        setPieceAtPosition(7, 0, new Rook(PieceColor::BLACK, 7, 0, BLACK_H_ROOK_ID));
        setPieceAtPosition(7, 7, new Rook(PieceColor::WHITE, 7, 7, WHITE_H_ROOK_ID));

        int id = PIECE_ID_START;
        const std::vector<PieceColor> colors{PieceColor::BLACK, PieceColor::WHITE};
        for (PieceColor color : colors) {
            int y = color == PieceColor::BLACK ? 0 : 7;

            setPieceAtPosition(1, y, new Knight(color, 1, y, id++));
            setPieceAtPosition(2, y, new Bishop(color, 2, y, id++));
            setPieceAtPosition(3, y, new Queen(color, 3, y, id++));
            setPieceAtPosition(5, y, new Bishop(color, 5, y, id++));
            setPieceAtPosition(6, y, new Knight(color, 6, y, id++));

            y += color == PieceColor::BLACK ? 1 : -1;
            for (int x = 0; x < 8; x++) {
                setPieceAtPosition(x, y, new Pawn(color, x, y, id++));
            }
        }

        updateGameState(getOppositeColor(getMovingColor()));
        createInitialZobristHash();
    }

    Tile* Board::getTile(int x, int y) {
        if (x < 0 || x > 7 || y < 0 || y > 7) {
            return nullptr;
        }

        return &board[x][y];
    }

    Tile* Board::getTileInDirection(int x, int y, Direction direction) {
        return getTile(x + DIRECTION_VALUES[direction].dx, y + DIRECTION_VALUES[direction].dy);
    }

    void
    Board::getTilesInDirection(std::vector<Tile*> &result, int x, int y, Direction direction, PieceColor movingColor, bool ignoreColor) {
        Tile* tile = getTileInDirection(x, y, direction);

        if (tile == nullptr) return;

        while (tile != nullptr) {
            Piece* piece = tile->getPiece();

            if (piece != nullptr) {
                if (ignoreColor || (piece->getPieceType() != PieceType::KING && piece->getColor() != movingColor)) {
                    result.push_back(tile);
                }

                return;
            }

            result.push_back(tile);
            tile = getTileInDirection(tile->getX(), tile->getY(), direction);
        }
    }

    void Board::print() {
        for (int y = 0; y < 8; y++) {
            for (auto &x : this->board) {
                Piece* piece = x[y].getPiece();

                if (piece) {
                    std::cout << piece->getSymbol() << " ";
                } else {
                    std::cout << ".. ";
                }
            }

            std::cout << std::endl;
        }

        std::cout << std::endl;
    }

    void Board::setPieceAtPosition(int x, int y, Piece* piece) {
        Tile* tile = getTile(x, y);

        if (piecePositions.contains(piece->getId())) {
            piecePositions.erase(piece->getId());
        }

        if (tile->getPiece()) {
            piecePositions.erase(tile->getPiece()->getId());
        }

        piecePositions.insert({piece->getId(), {x, y}});
        tile->setPiece(piece);
    }

    void Board::updateGameState(PieceColor movingColor) {
        if (movingColor == PieceColor::WHITE) {
            this->gameState = GameState::BLACK_TURN;
        } else {
            this->gameState = GameState::WHITE_TURN;
        }

        for (int i = 0; i < 8; i++) {
            Tile* tile = getTile(i, 2);
            Tile* tile2 = getTile(i, 5);

            if (tile->getEnPassantTarget()) {
                if (tile->getEnPassantTarget()->getColor() != movingColor) {
                    tile->setEnPassantTarget(nullptr);
                }
            }

            if (tile2->getEnPassantTarget()) {
                if (tile2->getEnPassantTarget()->getColor() != movingColor) {
                    tile2->setEnPassantTarget(nullptr);
                }
            }
        }
    }

    std::vector<Tile*> Board::removeMovesCausingCheck(std::vector<Tile*> &moves, Piece* &movingPiece) {
        std::vector<Tile*> result;
        result.reserve(moves.size());

        for (Tile* move : moves) {
            makeMove(move->getX(), move->getY(), movingPiece);

            if (!isKingChecked(movingPiece->getColor())) {
                result.push_back(move);
            }

            unmakeMove();
        }

        return result;
    }

    void Board::makeMove(int toX, int toY, Piece* &movingPiece) {
        TileLocation fromLoc = getPiecePosition(movingPiece->getId());
        Tile* fromTile = getTile(fromLoc.x, fromLoc.y);
        Tile* toTile = getTile(toX, toY);
        Piece* toPiece = toTile->getPiece();
        UndoData data;

        for (int i = 0; i < 8; i++) {
            Tile* tile = getTile(i, 2);
            Tile* tile2 = getTile(i, 5);

            data.enPassantTargets.emplace_back(tile->getX(), tile->getY(), tile->getEnPassantTarget());
            data.enPassantTargets.emplace_back(tile2->getX(), tile2->getY(), tile2->getEnPassantTarget());
        }

        data.gameState = getGameState();
        data.zobristHash = getZobristHash();
        data.halfMoveClock = halfMoveClock;
        data.hasBlackCastled = hasBlackCastled;
        data.hasWhiteCastled = hasWhiteCastled;

        data.originalPositions.push_back({movingPiece, {fromLoc.x, fromLoc.y}});
        data.hasMoved.emplace_back(movingPiece, movingPiece->getHasMoved());

        if (toPiece) {
            data.originalPositions.push_back({toPiece, {toTile->getX(), toTile->getY()}});
            data.hasMoved.emplace_back(toPiece, toPiece->getHasMoved());
        }

        if (toTile->getEnPassantTarget()) {
            if (toTile->getEnPassantTarget()->getColor() == PieceColor::WHITE) {
                data.originalPositions.push_back({toTile->getEnPassantTarget(), {toTile->getX(), 5}});
            } else {
                data.originalPositions.push_back({toTile->getEnPassantTarget(), {toTile->getX(), 4}});
            }
            data.hasMoved.emplace_back(toTile->getEnPassantTarget(), toTile->getEnPassantTarget()->getHasMoved());
        }

        if (movingPiece->getPieceType() == PieceType::PAWN) {
            if (toY == 0 || toY == 7) {
                data.promotionTile = getTile(toX, toY);
            }
        }

        undoData.push(data);

        if (movingPiece->getPieceType() == PieceType::PAWN && fromLoc.x != toX && toTile->getEnPassantTarget() != nullptr && toTile->getPiece() == nullptr
        && fromLoc.y == (movingPiece->getColor() == PieceColor::WHITE ? 3 : 4)) {
            // TODO: fix, prob broken
            handleEnPassant(movingPiece, toTile);
            // TODO: don't make from scratch
            createInitialZobristHash();
        } else if (toPiece && movingPiece->getPieceType() == PieceType::KING && toPiece->getPieceType() == PieceType::ROOK) {
            handleKingCastle(toY, movingPiece, fromTile, toTile, toPiece);

            if (movingPiece->getColor() == PieceColor::WHITE) {
                hasWhiteCastled = true;
            } else {
                hasBlackCastled = true;
            }

            // TODO: don't make from scratch
            createInitialZobristHash();
        } else {
            if (movingPiece->getPieceType() == PieceType::PAWN) {
                // TODO: handle other promotions(Field in pawn class?)
                if (toY == 0 || toY == 7) {
                    Queen* queen = new Queen(movingPiece->getColor(), toX, toY, movingPiece->getId());
                    movingPiece = queen;

                    removePieceFromPosition(toTile->getX(), toTile->getY());
                    movingPiece->setHasMoved(true);
                    setPieceAtPosition(toTile->getX(), toTile->getY(), movingPiece);
                    createInitialZobristHash();
                    return;
                }

                if (std::abs(fromLoc.y - toY) == 2) {
                    int enPassantY = movingPiece->getColor() == PieceColor::WHITE ? 5 : 2;
                    Tile* enPassantTile = getTile(toX, enPassantY);

                    enPassantTile->setEnPassantTarget(movingPiece);
                }
            }

            removePieceFromPosition(fromTile->getX(), fromTile->getY());
            movingPiece->setHasMoved(true);
            setPieceAtPosition(toTile->getX(), toTile->getY(), movingPiece);

            if (toPiece) {
                int index;
                int tileIndex = toX * 8 + toY;

                if (toPiece->getColor() == PieceColor::BLACK) {
                    index = toPiece->getPieceType() * 64 + tileIndex;
                } else {
                    index = (toPiece->getPieceType() + 6) * 64 + tileIndex;
                }

                zobristHash ^= zobristConstants[index];
                halfMoveClock = 0;
            } else {
                if (movingPiece->getPieceType() == PieceType::PAWN) {
                    halfMoveClock = 0;
                } else {
                    halfMoveClock += 1;
                }
            }

            {
                int index;
                int tileIndex = fromTile->getX() * 8 + fromTile->getY();

                if (movingPiece->getColor() == PieceColor::BLACK) {
                    index = movingPiece->getPieceType() * 64 + tileIndex;
                } else {
                    index = (movingPiece->getPieceType() + 6) * 64 + tileIndex;
                }

                zobristHash ^= zobristConstants[index];
            }

            {
                int index;
                int tileIndex = toX * 8 + toY;

                if (movingPiece->getColor() == PieceColor::BLACK) {
                    index = movingPiece->getPieceType() * 64 + tileIndex;
                } else {
                    index = (movingPiece->getPieceType() + 6) * 64 + tileIndex;
                }

                zobristHash ^= zobristConstants[index];
            }

            zobristHash ^= zobristConstants[12 * 64];
        }

        if (moveHistory.contains(getZobristHash())) {
            int amount = moveHistory.at(getZobristHash());

            moveHistory.erase(getZobristHash());
            moveHistory.insert({getZobristHash(), amount + 1});
        } else {
            moveHistory.insert({getZobristHash(), 1});
        }

        movesMade += 1;
        updateGameState(movingPiece->getColor());
    }

    void Board::handleEnPassant(Piece* movingPiece, Tile* toTile) {
        Piece* enPassantTarget = toTile->getEnPassantTarget();
        TileLocation fromLoc = getPiecePosition(movingPiece->getId());
        Tile* fromTile = getTile(fromLoc.x, fromLoc.y);
        TileLocation enPassantTargetLoc = getPiecePosition(enPassantTarget->getId());
        Tile* enPassantTargetTile = getTile(enPassantTargetLoc.x, enPassantTargetLoc.y);

        removePieceFromPosition(fromTile->getX(), fromTile->getY());
        removePieceFromPosition(enPassantTargetTile->getX(), enPassantTargetTile->getY());
        removePieceFromPosition(toTile->getX(), toTile->getY());
        setPieceAtPosition(toTile->getX(), toTile->getY(), movingPiece);
    }

    void Board::handleKingCastle(int y, Piece* king, Tile* fromTile, Tile* toTile, Piece* rook) {
        TileLocation rookLoc = getPiecePosition(rook->getId());
        int castleRookX;
        int castleKingX;

        if (rookLoc.x == 0) {
            castleRookX = 3;
            castleKingX = 2;
        } else {
            castleRookX = 5;
            castleKingX = 6;
        }

        Tile* castleRookTile = getTile(castleRookX, y);
        Tile* castleKingTile = getTile(castleKingX, y);
        removePieceFromPosition(fromTile->getX(), fromTile->getY());
        removePieceFromPosition(toTile->getX(), toTile->getY());
        king->setHasMoved(true);
        rook->setHasMoved(true);
        setPieceAtPosition(castleKingTile->getX(), castleKingTile->getY(), king);
        setPieceAtPosition(castleRookTile->getX(), castleRookTile->getY(), rook);
    }

    TileLocation Board::getPiecePosition(int id) {
        if (!piecePositions.contains(id)) {
            return {-5, -5};
        }

        return piecePositions.at(id);
    }

    GameState Board::getState() const {
        return gameState;
    }

    void Board::setState(GameState state) {
        Board::gameState = state;
    }

    std::vector<Piece*> Board::getPieces(PieceColor color) {
        std::vector<Piece*> pieces;
        pieces.reserve(16);

        for (auto &piecePosition : piecePositions) {
            TileLocation position = piecePosition.second;
            Tile* tile = getTile(position.x, position.y);
            Piece* piece = tile->getPiece();

            if (piece->getColor() == color) {
                pieces.push_back(piece);
            }
        }

        return pieces;
    }

    std::vector<Piece*> Board::getPieces() {
        std::vector<Piece*> pieces;
        pieces.reserve(32);

        for (auto &piecePosition : piecePositions) {
            TileLocation position = piecePosition.second;
            Tile* tile = getTile(position.x, position.y);

            pieces.push_back(tile->getPiece());
        }

        return pieces;
    }

    bool Board::isDraw() {
        PieceColor color = getMovingColor();

        if (isStalemate(color) || isInsufficientMaterial()
        || (moveHistory.contains(getZobristHash()) && moveHistory.at(getZobristHash()) >= 3) || halfMoveClock >= 100) {
            return true;
        }

        return false;
    }

    bool Board::isStalemate(PieceColor color) {
        std::vector<Tile*> legalMoves;
        legalMoves.reserve(10);

        for (Piece* &piece : getPieces(color)) {
            piece->getPseudoLegalMoves(legalMoves, this);

            if (!legalMoves.empty()) {
                legalMoves = removeMovesCausingCheck(legalMoves, piece);

                if (!legalMoves.empty()) {
                    return false;
                }
            }
        }

        return true;
    }

    void Board::unmakeMove() {
        if (undoData.empty()) {
            return;
        }

        UndoData move = undoData.top();
        undoData.pop();

        if (move.promotionTile) {
            if (move.promotionTile->getPiece()) {
                removePieceFromPosition(move.promotionTile->getX(), move.promotionTile->getY());
            }
        }

        for (std::pair<Piece*, TileLocation> entry : move.originalPositions) {
            Piece* piece = entry.first;
            int toX = entry.second.x;
            int toY = entry.second.y;
            TileLocation currentLoc = getPiecePosition(piece->getId());
            Tile* currentTile = getTile(currentLoc.x, currentLoc.y);

            if (currentTile) {
                removePieceFromPosition(currentTile->getX(), currentTile->getY());
            }

            setPieceAtPosition(toX, toY, piece);
        }

        for (std::pair<Piece*, bool> entry : move.hasMoved) {
            Piece* piece = entry.first;
            bool hasMoved = entry.second;

            piece->setHasMoved(hasMoved);
        }

        for (std::tuple<int, int, Piece*> entry : move.enPassantTargets) {
            int enPassantX = std::get<0>(entry);
            int enPassantY = std::get<1>(entry);
            Piece* piece = std::get<2>(entry);
            Tile* tile = getTile(enPassantX, enPassantY);

            tile->setEnPassantTarget(piece);
        }

        this->gameState = move.gameState;
        this->zobristHash = move.zobristHash;
        this->halfMoveClock = move.halfMoveClock;
        this->hasWhiteCastled = move.hasWhiteCastled;
        this->hasBlackCastled = move.hasBlackCastled;
        this->movesMade -= 1;
        updateGameState(getMovingColor());
    }

    GameState Board::getGameState() const {
        return gameState;
    }

    void Board::setGameState(GameState gameState) {
        this->gameState = gameState;
    }

    bool Board::isKingChecked(PieceColor color) {
        TileLocation loc = getPiecePosition(color == PieceColor::WHITE ? WHITE_KING_ID : BLACK_KING_ID);
        King* king = reinterpret_cast<King*>(getTile(loc.x, loc.y)->getPiece());

        return king->isChecked(this);
    }

    bool Board::orderMoves(const Move &a, const Move &b) {
        int scoreA = mvvlva(a.tile, a.piece->getColor());
        int scoreB = mvvlva(b.tile, b.piece->getColor());
        uint64_t zobristA = getZobristHashForMove(a.tile->getX(), a.tile->getY(), a.piece);
        uint64_t zobristB = getZobristHashForMove(b.tile->getX(), b.tile->getY(), b.piece);

        if (tt.isPVMove(a)) {
            scoreA += 50000 + tt.getPositionScore(zobristA);
        } else if (tt.isPositionInTable(zobristA)) {
            scoreA += 25000 + tt.getPositionScore(zobristA);
        } else if (scoreA > 0) {
            // Good capture
            scoreA += 10000;
        } else if (tt.isKillerMove(a)) {
            scoreA += 5000;
        }

        if (tt.isPVMove(b)) {
            scoreB += 50000 + tt.getPositionScore(zobristB);
        } else if (tt.isPositionInTable(zobristB)) {
            scoreB += 25000 + tt.getPositionScore(zobristB);
        } else if (scoreB > 0) {
            // Good capture
            scoreB += 10000;
        } else if (tt.isKillerMove(b)) {
            scoreB += 5000;
        }

        return scoreA > scoreB;
    }

    std::vector<Move> Board::getLegalMoves(PieceColor color, bool pseudoLegal) {
        std::vector<Move> legalMoves;
        legalMoves.reserve(64);

        for (Piece* &piece : this->getPieces(color)) {
            std::vector<Tile*> moves;
            moves.reserve(15);

            piece->getPseudoLegalMoves(moves, this);

            if (!pseudoLegal) {
                moves = removeMovesCausingCheck(moves, piece);
            }

            for (Tile* move : moves) {
                legalMoves.push_back({move, piece});
            }
        }

        std::sort(legalMoves.begin(), legalMoves.end(), [this](const Move &a, const Move &b){
            return orderMoves(a, b);
        });

        return legalMoves;
    }

    PieceColor Board::getWinner() {
        for (PieceColor color : {PieceColor::WHITE, PieceColor::BLACK}) {
            TileLocation loc = getPiecePosition(color == PieceColor::WHITE ? WHITE_KING_ID : BLACK_KING_ID);
            King* king = reinterpret_cast<King*>(getTile(loc.x, loc.y)->getPiece());

            if (king->isCheckMate(this)) {
                return getOppositeColor(color);
            }
        }

        return PieceColor::NONE;
    }

    PieceColor Board::getMovingColor() const {
        GameState state = getGameState();

        if (state == GameState::WHITE_TURN) {
            return PieceColor::WHITE;
        } else if (state == GameState::BLACK_TURN) {
            return PieceColor::BLACK;
        } else {
            return PieceColor::NONE;
        }
    }

    bool Board::makeStrMove(const std::string &move) {
        std::string from = move.substr(0, 2);
        std::string to = move.substr(2, 4);
        Tile* fromTile = getTileFromString(from);
        Tile* toTile = getTileFromString(to);

        if (fromTile == nullptr || toTile == nullptr) {
            return false;
        }

        Piece* piece = fromTile->getPiece();

        makeMove(toTile->getX(), toTile->getY(), piece);
        return true;
    }

    Tile* Board::getTileFromString(const std::string &tile) {
        std::string file = tile.substr(0, 1);
        std::string rank = tile.substr(1, 1);

        int x = file.at(0) - 'a';
        int y = 8 - stoi(rank);

        return getTile(x, y);
    }

    std::string Board::getFEN() {
        std::string fen;

        for (int y = 0; y < 8; y++) {
            int emptyTiles = 0;

            for (int x = 0; x < 8; x++) {
                Tile* tile = getTile(x, y);

                if (tile->getPiece() == nullptr) {
                    emptyTiles++;
                } else {
                    if (emptyTiles > 0) {
                        fen += std::to_string(emptyTiles);
                        emptyTiles = 0;
                    }

                    fen += tile->getPiece()->getFEN();
                }
            }

            if (emptyTiles > 0) {
                fen += std::to_string(emptyTiles);
            }

            if (y < 7) {
                fen += "/";
            }
        }

        fen += " ";
        if (getMovingColor() == PieceColor::WHITE) {
            fen += "w";
        } else {
            fen += "b";
        }

        // TODO: implement halfmove clock and fullmove counter
        fen += " " + getCastlingFEN() + " " + getEnPassantFEN() + " 0 1";

        return fen;
    }

    std::string Board::getCastlingFEN() {
        std::string castlingString;

        for (PieceColor color : {PieceColor::WHITE, PieceColor::BLACK}) {
            TileLocation kingLoc = getPiecePosition(color == PieceColor::WHITE ? WHITE_KING_ID : BLACK_KING_ID);
            TileLocation aRookLoc = getPiecePosition(color == PieceColor::WHITE ? WHITE_A_ROOK_ID : BLACK_A_ROOK_ID);
            TileLocation hRookLoc = getPiecePosition(color == PieceColor::WHITE ? WHITE_H_ROOK_ID : BLACK_H_ROOK_ID);
            Piece* king = getTile(kingLoc.x, kingLoc.y)->getPiece();
            Tile* aRookTile = getTile(aRookLoc.x, aRookLoc.y);
            Tile* hRookTile = getTile(hRookLoc.x, hRookLoc.y);

            if (king->getHasMoved()) {
                continue;
            }

            if (hRookTile) {
                if (!hRookTile->getPiece()->getHasMoved()) {
                    castlingString += color == PieceColor::WHITE ? "K" : "k";
                }
            }

            if (aRookTile) {
                if (!aRookTile->getPiece()->getHasMoved()) {
                    castlingString += color == PieceColor::WHITE ? "Q" : "q";
                }
            }
        }

        if (castlingString.empty()) {
            return "-";
        } else {
            return castlingString;
        }
    }

    std::string Board::getEnPassantFEN() {
        std::string enPassantString;

        for (int i = 0; i < 8; i++) {
            Tile* tile = getTile(i, 2);

            if (tile->getEnPassantTarget() != nullptr) {
                enPassantString += tile->getNotation();
            }
        }

        for (int i = 0; i < 8; i++) {
            Tile* tile = getTile(i, 5);

            if (tile->getEnPassantTarget() != nullptr) {
                enPassantString += tile->getNotation();
            }
        }

        if (enPassantString.empty()) {
            return "-";
        } else {
            return enPassantString;
        }
    }

    uint64_t Board::getWhiteTimeMsec() const {
        return whiteTimeMsec;
    }

    void Board::setWhiteTimeMsec(uint64_t whiteTimeMsec) {
        this->whiteTimeMsec = whiteTimeMsec;
    }

    uint64_t Board::getBlackTimeMsec() const {
        return blackTimeMsec;
    }

    void Board::setBlackTimeMsec(uint64_t blackTimeMsec) {
        this->blackTimeMsec = blackTimeMsec;
    }

    // TODO: implement en passant
    // TODO: clean up
    bool Board::setFromFEN(const std::string &fen) {
        // Reset the board
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                Tile tile = Tile{x, y};

                board[x][y] = tile;
            }
        }

        int y = 0;
        int x = 0;
        int spaces = 0;

        for (const char character : fen) {
            if (character == ' ') {
                spaces++;
                continue;
            }

            if (spaces == 0) {
                if (character == '/') {
                    y++;
                    x = 0;
                    continue;
                }

                if (x > 7 || y > 7) {
                    std::cout << "X or Y too high: x=" << x << ", y=" << y << std::endl;
                    return false;
                }

                if (character >= '1' && character <= '8') {
                    x += character - '0';
                    continue;
                }

                if (character >= 'A' && character <= 'z') {
                    setPieceFromFENChar(character, x, y);
                    x++;
                    continue;
                } else{
                    std::cout << "Invalid character!" << std::endl;
                    return false;
                }
            }

            if (spaces == 1) {
                gameState = character == 'w' ? GameState::WHITE_TURN : GameState::BLACK_TURN;
                continue;
            }

            if (spaces == 2) {
                if (character == '-') {
                    continue;
                }

                if (character == 'K') {
                    TileLocation whiteRookLoc = getPiecePosition(WHITE_H_ROOK_ID);
                    Piece* piece = getTile(whiteRookLoc.x, whiteRookLoc.y)->getPiece();
                    piece->setHasMoved(false);
                    continue;
                }

                if (character == 'Q') {
                    TileLocation whiteQueenLocation = getPiecePosition(WHITE_A_ROOK_ID);
                    Piece* piece = getTile(whiteQueenLocation.x, whiteQueenLocation.y)->getPiece();
                    piece->setHasMoved(false);
                    continue;
                }

                if (character == 'k') {
                    TileLocation blackKingLocation = getPiecePosition(BLACK_A_ROOK_ID);
                    Piece* piece = getTile(blackKingLocation.x, blackKingLocation.y)->getPiece();
                    piece->setHasMoved(false);
                    continue;
                }

                if (character == 'q') {
                    TileLocation blackQueenLocation = getPiecePosition(BLACK_H_ROOK_ID);
                    Piece* piece = getTile(blackQueenLocation.x, blackQueenLocation.y)->getPiece();
                    piece->setHasMoved(false);
                    continue;
                }

                continue;
            }
        }

        updateGameState(getOppositeColor(getMovingColor()));
        return true;
    }

    void Board::setPieceFromFENChar(char character, int x, int y) {
        int id = PIECE_ID_START;

        while (piecePositions.contains(id)) {
            id++;
        }

        // TODO: id system is hacky, figure out something better
        switch (character) {
            case 'P': {
                Pawn* pawn = new Pawn(PieceColor::WHITE, x, y, id);

                if (y != 6) {
                    pawn->setHasMoved(true);
                }

                setPieceAtPosition(x, y, pawn);
                break;
            }
            case 'p':{
                Pawn* pawn = new Pawn(PieceColor::BLACK, x, y, id);

                if (y != 1) {
                    pawn->setHasMoved(true);
                }

                setPieceAtPosition(x, y, pawn);
                break;
            }
            case 'N':
                setPieceAtPosition(x, y, new Knight(PieceColor::WHITE, x, y, id));
                break;
            case 'n':
                setPieceAtPosition(x, y, new Knight(PieceColor::BLACK, x, y, id));
                break;
            case 'B':
                setPieceAtPosition(x, y, new Bishop(PieceColor::WHITE, x, y, id));
                break;
            case 'b':
                setPieceAtPosition(x, y, new Bishop(PieceColor::BLACK, x, y, id));
                break;
            case 'R':
                if (x <= 4) {
                    Rook* rook = new Rook(PieceColor::WHITE, x, y, WHITE_A_ROOK_ID);

                    rook->setHasMoved(true);
                    setPieceAtPosition(x, y, rook);
                } else {
                    Rook* rook = new Rook(PieceColor::WHITE, x, y, WHITE_H_ROOK_ID);

                    rook->setHasMoved(true);
                    setPieceAtPosition(x, y, rook);
                }
                break;
            case 'r':
                if (x <= 4) {
                    Rook* rook = new Rook(PieceColor::BLACK, x, y, BLACK_A_ROOK_ID);

                    rook->setHasMoved(true);
                    setPieceAtPosition(x, y, rook);
                } else {
                    Rook* rook = new Rook(PieceColor::BLACK, x, y, BLACK_H_ROOK_ID);

                    rook->setHasMoved(true);
                    setPieceAtPosition(x, y, rook);
                }
                break;
            case 'Q':
                setPieceAtPosition(x, y, new Queen(PieceColor::WHITE, x, y, id));
                break;
            case 'q':
                setPieceAtPosition(x, y, new Queen(PieceColor::BLACK, x, y, id));
                break;
            case 'K': {
                King* king = new King(PieceColor::WHITE, x, y, WHITE_KING_ID);

                setPieceAtPosition(x, y, king);
                break;
            }
            case 'k': {
                King* king = new King(PieceColor::BLACK, x, y, BLACK_KING_ID);

                setPieceAtPosition(x, y, king);
                break;
            }
        }
    }

    int Board::mvvlva(Tile* tile, PieceColor attackingColor) {
        int score = 0;
/*        int aggressorWeight = 99999999;
        Piece* piece = tile->getPiece();

        if (piece != nullptr) {
            for (Piece* attacker : tile->getAttackersByColor(attackingColor)) {
                if (attacker->getPieceType() != PieceType::KING && attacker->getWeight(this) < aggressorWeight) {
                    aggressorWeight = attacker->getWeight(this);
                }
            }

            score = piece->getWeight(this) - aggressorWeight;
        }*/

        return score;
    }

    bool Board::isInsufficientMaterial() {
        std::vector<Piece*> pieces = getPieces();
        int pieceCount = pieces.size();

        if (pieceCount > 4) {
            return false;
        }

        if (pieceCount == 2) {
            return true;
        }

        if (pieceCount == 3) {
            for (Piece* piece : pieces) {
                if (piece->getPieceType() == PieceType::KNIGHT || piece->getPieceType() == PieceType::BISHOP) {
                    return true;
                }
            }
        }

        if (pieceCount == 4) {
            for (Piece* piece : pieces) {
                if (!(piece->getPieceType() == PieceType::BISHOP || piece->getPieceType() == PieceType::KING)) {
                    return false;
                }
            }

            return true;
        }

        return false;
    }

    int Board::getMovesMade() const {
        return movesMade;
    }

    void Board::createInitialZobristHash() {
        zobristHash = 0;
        int blackToMoveIndex = 12 * 64;
        int castlingIndex = blackToMoveIndex + 1;

        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
                Tile* tile = getTile(x, y);
                Piece* piece = tile->getPiece();

                if (piece != nullptr) {
                    int tileIndex = x * 8 + y;
                    int index;
                    
                    if (piece->getColor() == PieceColor::BLACK) {
                        index = piece->getPieceType() * 64 + tileIndex;
                    } else {
                        index = (piece->getPieceType() + 6) * 64 + tileIndex;
                    }

                    zobristHash ^= zobristConstants[index];
                }
            }
        }

        if (getMovingColor() == PieceColor::BLACK) {
            zobristHash ^= zobristConstants[blackToMoveIndex];
        }

        for (PieceColor color : {PieceColor::WHITE, PieceColor::BLACK}) {
            TileLocation kingLoc = getPiecePosition(color == PieceColor::WHITE ? WHITE_KING_ID : BLACK_KING_ID);
            TileLocation aRookLoc = getPiecePosition(color == PieceColor::WHITE ? WHITE_A_ROOK_ID : BLACK_A_ROOK_ID);
            TileLocation hRookLoc = getPiecePosition(color == PieceColor::WHITE ? WHITE_H_ROOK_ID : BLACK_H_ROOK_ID);
            Piece* king = getTile(kingLoc.x, kingLoc.y)->getPiece();
            Tile* aRookTile = getTile(aRookLoc.x, aRookLoc.y);
            Tile* hRookTile = getTile(hRookLoc.x, hRookLoc.y);

            if (king->getHasMoved()) {
                continue;
            }

            if (hRookTile) {
                if (!hRookTile->getPiece()->getHasMoved()) {
                    zobristHash ^= color == PieceColor::WHITE ? castlingIndex : castlingIndex + 2;
                }
            }

            if (aRookTile) {
                if (!aRookTile->getPiece()->getHasMoved()) {
                    zobristHash ^= color == PieceColor::WHITE ? castlingIndex + 1 : castlingIndex + 3;
                }
            }
        }

        for (int i = 0; i < 8; i++) {
            Tile* tile = getTile(i, 2);
            Tile* tile2 = getTile(i, 5);

            if (tile->getEnPassantTarget() != nullptr) {
                zobristHash ^= zobristConstants[i];
            }

            if (tile2->getEnPassantTarget() != nullptr) {
                zobristHash ^= zobristConstants[i + 8];
            }
        }
    }

    uint64_t Board::getZobristHash() {
        return zobristHash;
    }

    uint64_t Board::getZobristHashForMove(int toX, int toY, Piece* movingPiece) {
        TileLocation fromLoc = getPiecePosition(movingPiece->getId());
        Tile* toTile = getTile(toX, toY);
        Piece* toPiece = toTile->getPiece();
        uint64_t hash = zobristHash;

        if (toPiece) {
            int index;
            int tileIndex = toX * 8 + toY;

            if (toPiece->getColor() == PieceColor::BLACK) {
                index = toPiece->getPieceType() * 64 + tileIndex;
            } else {
                index = (toPiece->getPieceType() + 6) * 64 + tileIndex;
            }

            zobristHash ^= zobristConstants[index];
        }

        {
            int index;
            int tileIndex = fromLoc.x * 8 + fromLoc.y;

            if (movingPiece->getColor() == PieceColor::BLACK) {
                index = movingPiece->getPieceType() * 64 + tileIndex;
            } else {
                index = (movingPiece->getPieceType() + 6) * 64 + tileIndex;
            }

            hash ^= zobristConstants[index];
        }

        {
            int index;
            int tileIndex = toX * 8 + toY;

            if (movingPiece->getColor() == PieceColor::BLACK) {
                index = movingPiece->getPieceType() * 64 + tileIndex;
            } else {
                index = (movingPiece->getPieceType() + 6) * 64 + tileIndex;
            }

            hash ^= zobristConstants[index];
        }

        hash ^= zobristConstants[12 * 64];
        return hash;
    }

    bool Board::isInCenter(int x, int y) {
        return x >= 3 && x <= 4 && y >= 3 && y <= 4;
    }

    bool Board::isInExtendedCenter(int x, int y) {
        return x >= 2 && x <= 5 && y >= 2 && y <= 5;
    }

    bool Board::getHasBlackCastled() const {
        return hasBlackCastled;
    }

    bool Board::getHasWhiteCastled() const {
        return hasWhiteCastled;
    }

    bool Board::hasCastled(PieceColor color) {
        if (color == PieceColor::BLACK) {
            return hasBlackCastled;
        } else {
            return hasWhiteCastled;
        }
    }

    bool Board::isTileAttackedByColor(Tile* targetSquare, PieceColor attackingColor) {
        for (Direction direction : ALL_DIRECTIONS) {
            std::vector<Tile*> attackedSquares{8};
            getTilesInDirection(attackedSquares, targetSquare->getX(), targetSquare->getY(), direction,
                                getOppositeColor(attackingColor), false);

            for (Tile* square : attackedSquares) {
                if (square != nullptr) {
                    Piece* piece = square->getPiece();

                    if (piece != nullptr) {
                        if (piece->getPieceType() == PieceType::PAWN || piece->getPieceType() == PieceType::KING) {
                            if (!(std::abs(targetSquare->getX() - square->getX()) <= 1 &&
                                  std::abs(targetSquare->getY() - square->getY()) <= 1)) {
                                continue;
                            }
                        }

                        if (piece->getColor() == attackingColor) {
                            if (canPieceAttackDirection(piece, direction)) {
                                return true;
                            }
                        }
                    }
                }
            }
        }

        for (DirectionValue directionValue : knightDirections) {
            Tile* square = getTile(targetSquare->getX() + directionValue.dx, targetSquare->getY() + directionValue.dy);

            if (square != nullptr) {
                Piece *piece = square->getPiece();

                if (piece != nullptr && piece->getColor() == attackingColor &&
                    piece->getPieceType() == PieceType::KNIGHT) {
                    return true;
                }
            }
        }

        return false;
    }

    bool Board::canPieceAttackDirection(Piece* piece, Direction direction) {
        switch(piece->getPieceType()) {
            case PAWN:
                if (piece->getColor() == PieceColor::BLACK) {
                    return std::find(BLACK_PAWN_ATTACK_DIRECTIONS.begin(), BLACK_PAWN_ATTACK_DIRECTIONS.end(), direction) != BLACK_PAWN_ATTACK_DIRECTIONS.end();
                } else {
                    return std::find(WHITE_PAWN_ATTACK_DIRECTIONS.begin(), WHITE_PAWN_ATTACK_DIRECTIONS.end(), direction) != WHITE_PAWN_ATTACK_DIRECTIONS.end();
                }
            case KNIGHT:
                // Has to be handled differently
                return false;
            case BISHOP:
                return std::find(BISHOP_DIRECTIONS.begin(), BISHOP_DIRECTIONS.end(), direction) != BISHOP_DIRECTIONS.end();
            case ROOK:
                return std::find(ROOK_DIRECTIONS.begin(), ROOK_DIRECTIONS.end(), direction) != ROOK_DIRECTIONS.end();
            case QUEEN:
            case KING:
                return true;
        }
    }

    void Board::removePieceFromPosition(int x, int y) {
        Tile* tile = getTile(x, y);

        if (tile->getPiece() && piecePositions.contains(tile->getPiece()->getId())) {
            piecePositions.erase(tile->getPiece()->getId());
        }

        tile->setPiece(nullptr);
    }
}