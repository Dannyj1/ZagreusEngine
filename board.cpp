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

        setPieceAtPosition(4, 0, std::make_shared<King>(PieceColor::BLACK, 4, 0, BLACK_KING_ID));
        setPieceAtPosition(4, 7, std::make_shared<King>(PieceColor::WHITE, 4, 7, WHITE_KING_ID));
        setPieceAtPosition(0, 0, std::make_shared<Rook>(PieceColor::BLACK, 0, 0, BLACK_A_ROOK_ID));
        setPieceAtPosition(0, 7, std::make_shared<Rook>(PieceColor::WHITE, 0, 7, WHITE_A_ROOK_ID));
        setPieceAtPosition(7, 0, std::make_shared<Rook>(PieceColor::BLACK, 7, 0, BLACK_H_ROOK_ID));
        setPieceAtPosition(7, 7, std::make_shared<Rook>(PieceColor::WHITE, 7, 7, WHITE_H_ROOK_ID));

        int id = PIECE_ID_START;
        const std::vector<PieceColor> colors{PieceColor::BLACK, PieceColor::WHITE};
        for (PieceColor color : colors) {
            int y = color == PieceColor::BLACK ? 0 : 7;

            setPieceAtPosition(1, y, std::make_shared<Knight>(color, 1, y, id++));
            setPieceAtPosition(2, y, std::make_shared<Bishop>(color, 2, y, id++));
            setPieceAtPosition(3, y, std::make_shared<Queen>(color, 3, y, id++));
            setPieceAtPosition(5, y, std::make_shared<Bishop>(color, 5, y, id++));
            setPieceAtPosition(6, y, std::make_shared<Knight>(color, 6, y, id++));

            y += color == PieceColor::BLACK ? 1 : -1;
            for (int x = 0; x < 8; x++) {
                setPieceAtPosition(x, y, std::make_shared<Pawn>(color, x, y, id++));
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

        while (tile != nullptr) {
            std::shared_ptr<Piece> piece = tile->getPiece();

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
                std::shared_ptr<Piece> piece = x[y].getPiece();

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

    void Board::setPieceAtPosition(int x, int y, const std::shared_ptr<Piece> &piece) {
        if (piecePositions.contains(piece->getId())) {
            piecePositions.erase(piece->getId());
        }

        piecePositions.insert({piece->getId(), {x, y}});
        this->board[x][y].setPiece(piece);
    }

    void Board::updateGameState(PieceColor movingColor) {
        std::vector<std::shared_ptr<Piece>> attackers;
        attackers.reserve(32);

        for (auto &x : this->board) {
            for (auto &tile : x) {
                tile.clearAttackers();

                if (tile.getPiece()) {
                    attackers.push_back(tile.getPiece());
                }
            }
        }

        for (const std::shared_ptr<Piece> &piece : attackers) {
            piece->setAttackedTiles(this);
        }

        if (movingColor == PieceColor::WHITE) {
            this->gameState = GameState::BLACK_TURN;
        } else {
            this->gameState = GameState::WHITE_TURN;
        }
    }

    std::vector<Tile*> Board::removeMovesCausingCheck(std::vector<Tile*> &moves, std::shared_ptr<Piece> &movingPiece) {
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

    void Board::makeMove(int toX, int toY, std::shared_ptr<Piece> &movingPiece) {
        TileLocation fromLoc = piecePositions.at(movingPiece->getId());
        Tile* fromTile = getTile(fromLoc.x, fromLoc.y);
        Tile* toTile = getTile(toX, toY);
        std::shared_ptr<Piece> toPiece = toTile->getPiece();
        UndoData data;

        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
                Tile* tile = getTile(x, y);
                std::shared_ptr<Piece> piece = tile->getPiece();

                if (piece) {
                    data.originalPositions.insert({piece, {x, y}});
                    data.hasMoved.insert({piece->getId(), piece->getHasMoved()});
                }

                if (y == 2 || y == 5) {
                    data.enPassantTargets.insert({{tile->getX(), tile->getY()}, tile->getEnPassantTarget()});
                }
            }
        }

        data.gameState = getGameState();
        data.zobristHash = getZobristHash();
        data.halfMoveClock = halfMoveClock;
        data.hasBlackCastled = hasBlackCastled;
        data.hasWhiteCastled = hasWhiteCastled;
        undoData.push(data);

        if (movingPiece->getPieceType() == PieceType::PAWN && fromLoc.x != toX && toTile->getEnPassantTarget() != nullptr && toTile->getPiece() == nullptr
        && fromLoc.y == (movingPiece->getColor() == PieceColor::WHITE ? 3 : 4)) {
            handleEnPassant(movingPiece, toTile);
            // TODO: don't make from scratch
            updateGameState(movingPiece->getColor());
            createInitialZobristHash();
        } else if (toPiece && movingPiece->getPieceType() == PieceType::KING && toPiece->getPieceType() == PieceType::ROOK) {
            handleKingCastle(toY, movingPiece, fromTile, toTile, toPiece);

            if (movingPiece->getColor() == PieceColor::WHITE) {
                hasWhiteCastled = true;
            } else {
                hasBlackCastled = true;
            }

            // TODO: don't make from scratch
            updateGameState(movingPiece->getColor());
            createInitialZobristHash();
        } else {
            std::vector<std::shared_ptr<Piece>> attackers { movingPiece };
            attackers.reserve(32);

            for (const std::shared_ptr<Piece> &piece : toTile->getAttackers()) {
                attackers.push_back(piece);
            }

            std::vector<Tile*> attackedTiles;
            attackers.reserve(32);

            for (const std::shared_ptr<Piece> &attacker : attackers) {
                attacker->getAttackedTiles(attackedTiles, this);
            }

            for (Tile* tile : attackedTiles) {
                if (tile != toTile) {
                    tile->clearAttackers();
                }
            }

            if (movingPiece->getPieceType() == PieceType::PAWN) {
                // TODO: handle other promotions(Field in pawn class?)
                if (toY == 0 || toY == 7) {
                    std::shared_ptr<Queen> queen = std::make_shared<Queen>(movingPiece->getColor(), toX, toY, movingPiece->getId());
                    movingPiece = queen;

                    fromTile->setPiece(nullptr);
                    movingPiece->setHasMoved(true);
                    toTile->setPiece(movingPiece);
                    piecePositions.erase(movingPiece->getId());
                    piecePositions.insert({movingPiece->getId(), {toX, toY}});
                    createInitialZobristHash();
                    return;
                }

                if (std::abs(fromLoc.y - toY) == 2) {
                    int enPassantY = movingPiece->getColor() == PieceColor::WHITE ? 5 : 2;
                    Tile* enPassantTile = getTile(toX, enPassantY);

                    enPassantTile->setEnPassantTarget(movingPiece);
                }
            }

            fromTile->setPiece(nullptr);
            movingPiece->setHasMoved(true);
            toTile->setPiece(movingPiece);
            piecePositions.erase(movingPiece->getId());
            piecePositions.insert({movingPiece->getId(), {toX, toY}});

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

            for (const std::shared_ptr<Piece> &piece : fromTile->getAttackers()) {
                attackers.push_back(piece);
            }

            for (const std::shared_ptr<Piece> &attacker : attackers) {
                if (std::find(attackers.begin(), attackers.end(), attacker) == attackers.end()) {
                    attackers.push_back(attacker);
                }
            }

            for (const std::shared_ptr<Piece> &attacker : attackers) {
                attacker->getAttackedTiles(attackedTiles, this);

                for (Tile* tile : attackedTiles) {
                    tile->addAttacker(attacker);
                }

                attackers.clear();
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

        movesMade += 1;

        if (moveHistory.contains(getZobristHash())) {
            int amount = moveHistory.at(getZobristHash());

            moveHistory.erase(getZobristHash());
            moveHistory.insert({getZobristHash(), amount + 1});
        } else {
            moveHistory.insert({getZobristHash(), 1});
        }
    }

    void Board::handleEnPassant(const std::shared_ptr<Piece> &movingPiece, Tile* toTile) {
        std::shared_ptr<Piece> enPassantTarget = toTile->getEnPassantTarget();
        TileLocation fromLoc = piecePositions.at(movingPiece->getId());
        Tile* fromTile = getTile(fromLoc.x, fromLoc.y);
        TileLocation enPassantTargetLoc = piecePositions.at(enPassantTarget->getId());
        Tile* enPassantTargetTile = getTile(enPassantTargetLoc.x, enPassantTargetLoc.y);

        piecePositions.erase(enPassantTarget->getId());
        fromTile->setPiece(nullptr);
        enPassantTargetTile->setPiece(nullptr);
        toTile->setEnPassantTarget(nullptr);
        toTile->setPiece(movingPiece);
        piecePositions.erase(movingPiece->getId());
        piecePositions.insert({movingPiece->getId(), {toTile->getX(), toTile->getY()}});
    }

    void Board::handleKingCastle(int y, const std::shared_ptr<Piece> &king, Tile* fromTile, Tile* toTile, const std::shared_ptr<Piece> &rook) {
        TileLocation rookLoc = piecePositions.at(rook->getId());
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
        fromTile->setPiece(nullptr);
        toTile->setPiece(nullptr);
        king->setHasMoved(true);
        rook->setHasMoved(true);
        castleKingTile->setPiece(king);
        castleRookTile->setPiece(rook);
        piecePositions.erase(king->getId());
        piecePositions.erase(rook->getId());
        piecePositions.insert({king->getId(), {castleKingX, y}});
        piecePositions.insert({rook->getId(), {castleRookX, y}});
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

    std::vector<std::shared_ptr<Piece>> Board::getPieces(PieceColor color) {
        std::vector<std::shared_ptr<Piece>> pieces;
        pieces.reserve(16);

        for (auto &piecePosition : piecePositions) {
            TileLocation position = piecePosition.second;
            Tile* tile = getTile(position.x, position.y);
            std::shared_ptr<Piece> piece = tile->getPiece();

            if (piece->getColor() == color) {
                pieces.push_back(piece);
            }
        }

        return pieces;
    }

    std::vector<std::shared_ptr<Piece>> Board::getPieces() {
        std::vector<std::shared_ptr<Piece>> pieces;
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

        for (std::shared_ptr<Piece> &piece : getPieces(color)) {
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

        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
                Tile* tile = getTile(x, y);
                tile->setPiece(nullptr);
                tile->setEnPassantTarget(nullptr);
            }
        }

        for (std::pair<std::shared_ptr<Piece>, TileLocation> entry : move.originalPositions) {
            std::shared_ptr<Piece> piece = entry.first;
            int toX = entry.second.x;
            int toY = entry.second.y;

            piece->setHasMoved(move.hasMoved.at(piece->getId()));
            getTile(toX, toY)->setPiece(piece);
            piecePositions.erase(piece->getId());
            piecePositions.insert({piece->getId(), {toX, toY}});
        }

        for (std::pair<std::pair<int, int>, std::shared_ptr<Piece>> entry : move.enPassantTargets) {
            int enPassantX = entry.first.first;
            int enPassantY = entry.first.second;
            std::shared_ptr<Piece> piece = entry.second;
            Tile* tile = getTile(enPassantX, enPassantY);

            tile->setEnPassantTarget(piece);
        }

        this->gameState = move.gameState;
        this->zobristHash = move.zobristHash;
        this->halfMoveClock = move.halfMoveClock;
        this->hasWhiteCastled = move.hasWhiteCastled;
        this->hasBlackCastled = move.hasBlackCastled;
        this->movesMade -= 1;
    }

    GameState Board::getGameState() const {
        return gameState;
    }

    void Board::setGameState(GameState gameState) {
        this->gameState = gameState;
    }

    bool Board::isKingChecked(PieceColor color) {
        TileLocation loc = getPiecePosition(color == PieceColor::WHITE ? WHITE_KING_ID : BLACK_KING_ID);
        std::shared_ptr<King> king = std::reinterpret_pointer_cast<King>(getTile(loc.x, loc.y)->getPiece());

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

        for (std::shared_ptr<Piece> &piece : this->getPieces(color)) {
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
            std::shared_ptr<King> king = std::reinterpret_pointer_cast<King>(getTile(loc.x, loc.y)->getPiece());

            if (king->isCheckMate(this)) {
                return getOppositeColor(color);
            }
        }

        return PieceColor::NONE;
    }

    void Board::setAttackedTilesInDirection(int x, int y, const std::shared_ptr<Piece> &attackingPiece, Direction direction, PieceColor movingColor, bool ignoreColor) {
        Tile* tile = getTileInDirection(x, y, direction);

        while (tile != nullptr) {
            std::shared_ptr<Piece> piece = tile->getPiece();

            if (piece != nullptr) {
                if (ignoreColor || (piece->getPieceType() != PieceType::KING && piece->getColor() != movingColor)) {
                    tile->addAttacker(attackingPiece);
                }

                return;
            }

            tile->addAttacker(attackingPiece);
            tile = getTileInDirection(tile->getX(), tile->getY(), direction);
        }
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

        std::shared_ptr<Piece> piece = fromTile->getPiece();

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
            std::shared_ptr<Piece> king = getTile(kingLoc.x, kingLoc.y)->getPiece();
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
    bool Board::setFromFEN(std::string fen) {
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
                    std::shared_ptr<Piece> piece = getTile(whiteRookLoc.x, whiteRookLoc.y)->getPiece();
                    piece->setHasMoved(false);
                    continue;
                }

                if (character == 'Q') {
                    TileLocation whiteQueenLocation = getPiecePosition(WHITE_A_ROOK_ID);
                    std::shared_ptr<Piece> piece = getTile(whiteQueenLocation.x, whiteQueenLocation.y)->getPiece();
                    piece->setHasMoved(false);
                    continue;
                }

                if (character == 'k') {
                    TileLocation blackKingLocation = getPiecePosition(BLACK_A_ROOK_ID);
                    std::shared_ptr<Piece> piece = getTile(blackKingLocation.x, blackKingLocation.y)->getPiece();
                    piece->setHasMoved(false);
                    continue;
                }

                if (character == 'q') {
                    TileLocation blackQueenLocation = getPiecePosition(BLACK_H_ROOK_ID);
                    std::shared_ptr<Piece> piece = getTile(blackQueenLocation.x, blackQueenLocation.y)->getPiece();
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
                std::shared_ptr<Pawn> pawn = std::make_shared<Pawn>(PieceColor::WHITE, x, y, id);

                if (y != 6) {
                    pawn->setHasMoved(true);
                }

                setPieceAtPosition(x, y, pawn);
                break;
            }
            case 'p':{
                std::shared_ptr<Pawn> pawn = std::make_shared<Pawn>(PieceColor::BLACK, x, y, id);

                if (y != 1) {
                    pawn->setHasMoved(true);
                }

                setPieceAtPosition(x, y, pawn);
                break;
            }
            case 'N':
                setPieceAtPosition(x, y, std::make_shared<Knight>(PieceColor::WHITE, x, y, id));
                break;
            case 'n':
                setPieceAtPosition(x, y, std::make_shared<Knight>(PieceColor::BLACK, x, y, id));
                break;
            case 'B':
                setPieceAtPosition(x, y, std::make_shared<Bishop>(PieceColor::WHITE, x, y, id));
                break;
            case 'b':
                setPieceAtPosition(x, y, std::make_shared<Bishop>(PieceColor::BLACK, x, y, id));
                break;
            case 'R':
                if (x <= 4) {
                    std::shared_ptr<Rook> rook = std::make_shared<Rook>(PieceColor::WHITE, x, y, WHITE_A_ROOK_ID);

                    rook->setHasMoved(true);
                    setPieceAtPosition(x, y, rook);
                } else {
                    std::shared_ptr<Rook> rook = std::make_shared<Rook>(PieceColor::WHITE, x, y, WHITE_H_ROOK_ID);

                    rook->setHasMoved(true);
                    setPieceAtPosition(x, y, rook);
                }
                break;
            case 'r':
                if (x <= 4) {
                    std::shared_ptr<Rook> rook = std::make_shared<Rook>(PieceColor::BLACK, x, y, BLACK_A_ROOK_ID);

                    rook->setHasMoved(true);
                    setPieceAtPosition(x, y, rook);
                } else {
                    std::shared_ptr<Rook> rook = std::make_shared<Rook>(PieceColor::BLACK, x, y, BLACK_H_ROOK_ID);

                    rook->setHasMoved(true);
                    setPieceAtPosition(x, y, rook);
                }
                break;
            case 'Q':
                setPieceAtPosition(x, y, std::make_shared<Queen>(PieceColor::WHITE, x, y, id));
                break;
            case 'q':
                setPieceAtPosition(x, y, std::make_shared<Queen>(PieceColor::BLACK, x, y, id));
                break;
            case 'K': {
                std::shared_ptr<King> king = std::make_shared<King>(PieceColor::WHITE, x, y, WHITE_KING_ID);

                setPieceAtPosition(x, y, king);
                break;
            }
            case 'k': {
                std::shared_ptr<King> king = std::make_shared<King>(PieceColor::BLACK, x, y, BLACK_KING_ID);

                setPieceAtPosition(x, y, king);
                break;
            }
        }
    }

    int Board::mvvlva(Tile* tile, PieceColor attackingColor) {
        int score = 0;
        int aggressorWeight = 99999999;
        std::shared_ptr<Piece> piece = tile->getPiece();

        if (piece != nullptr) {
            for (const std::shared_ptr<Piece> &attacker : tile->getAttackersByColor(attackingColor)) {
                if (attacker->getPieceType() != PieceType::KING && attacker->getWeight(this) < aggressorWeight) {
                    aggressorWeight = attacker->getWeight(this);
                }
            }

            score = piece->getWeight(this) - aggressorWeight;
        }

        return score;
    }

    bool Board::isInsufficientMaterial() {
        std::vector<std::shared_ptr<Piece>> pieces = getPieces();
        int pieceCount = pieces.size();

        if (pieceCount > 4) {
            return false;
        }

        if (pieceCount == 2) {
            return true;
        }

        if (pieceCount == 3) {
            for (const std::shared_ptr<Piece> &piece : pieces) {
                if (piece->getPieceType() == PieceType::KNIGHT || piece->getPieceType() == PieceType::BISHOP) {
                    return true;
                }
            }
        }

        if (pieceCount == 4) {
            for (const std::shared_ptr<Piece> &piece : pieces) {
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
                std::shared_ptr<Piece> piece = tile->getPiece();

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
            std::shared_ptr<Piece> king = getTile(kingLoc.x, kingLoc.y)->getPiece();
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

    uint64_t Board::getZobristHashForMove(int toX, int toY, const std::shared_ptr<Piece> &movingPiece) {
        TileLocation fromLoc = piecePositions.at(movingPiece->getId());
        Tile* toTile = getTile(toX, toY);
        std::shared_ptr<Piece> toPiece = toTile->getPiece();
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

    void Board::removeAttackedTiles(Tile* fromTile, Tile* toTile, std::shared_ptr<Piece> &movingPiece) {
        std::vector<std::shared_ptr<Piece>> attackers { movingPiece };
        attackers.reserve(32);

        for (const std::shared_ptr<Piece> &piece : toTile->getAttackers()) {
            attackers.push_back(piece);
        }

        std::vector<Tile*> attackedTiles;
        attackers.reserve(32);

        for (const std::shared_ptr<Piece> &attacker : attackers) {
            attacker->getAttackedTiles(attackedTiles, this);
        }

        for (Tile* tile : attackedTiles) {
            if (tile != toTile) {
                tile->clearAttackers();
            }
        }

        for (const std::shared_ptr<Piece> &piece : fromTile->getAttackers()) {
            attackers.push_back(piece);
        }

        for (const std::shared_ptr<Piece> &attacker : attackers) {
            if (std::find(attackers.begin(), attackers.end(), attacker) == attackers.end()) {
                attackers.push_back(attacker);
            }
        }

        for (const std::shared_ptr<Piece> &attacker : attackers) {
            attacker->getAttackedTiles(attackedTiles, this);

            for (Tile* tile : attackedTiles) {
                tile->addAttacker(attacker);
            }

            attackers.clear();
        }
    }
}