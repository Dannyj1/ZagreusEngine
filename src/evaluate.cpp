/*
 This file is part of Zagreus.

 Zagreus is a UCI chess engine
 Copyright (C) 2023  Danny Jelsma

 Zagreus is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published
 by the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Zagreus is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with Zagreus.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "evaluate.h"
#include "features.h"

namespace Zagreus {
    Evaluation::Evaluation(Bitboard &bitboard) : bitboard(bitboard) {
        uint64_t whitePawns = bitboard.getPieceBoard(PieceType::WHITE_PAWN);
        while (whitePawns) {
            int square = popLsb(whitePawns);
            uint64_t attacks = bitboard.getPawnAttacks<PieceColor::WHITE>(square);

            attacksByPiece[PieceType::WHITE_PAWN] |= attacks;
            attacksByColor[PieceColor::WHITE] |= attacks;
            combinedAttacks |= attacks;
            attacksFrom[square] |= attacks;
        }

        uint64_t whiteKnights = bitboard.getPieceBoard(PieceType::WHITE_KNIGHT);
        while (whiteKnights) {
            int square = popLsb(whiteKnights);
            uint64_t attacks = bitboard.getKnightAttacks(square);

            attacksByPiece[PieceType::WHITE_KNIGHT] |= attacks;
            attacksByColor[PieceColor::WHITE] |= attacks;
            combinedAttacks |= attacks;
            attacksFrom[square] |= attacks;
        }

        uint64_t whiteBishops = bitboard.getPieceBoard(PieceType::WHITE_BISHOP);
        while (whiteBishops) {
            int square = popLsb(whiteBishops);
            uint64_t attacks = bitboard.getBishopAttacks(square);

            attacksByPiece[PieceType::WHITE_BISHOP] |= attacks;
            attacksByColor[PieceColor::WHITE] |= attacks;
            combinedAttacks |= attacks;
            attacksFrom[square] |= attacks;
        }

        uint64_t whiteRooks = bitboard.getPieceBoard(PieceType::WHITE_ROOK);
        while (whiteRooks) {
            int square = popLsb(whiteRooks);
            uint64_t attacks = bitboard.getRookAttacks(square);

            attacksByPiece[PieceType::WHITE_ROOK] |= attacks;
            attacksByColor[PieceColor::WHITE] |= attacks;
            combinedAttacks |= attacks;
            attacksFrom[square] |= attacks;
        }

        uint64_t whiteQueens = bitboard.getPieceBoard(PieceType::WHITE_QUEEN);
        while (whiteQueens) {
            int square = popLsb(whiteQueens);
            uint64_t attacks = bitboard.getQueenAttacks(square);

            attacksByPiece[PieceType::WHITE_QUEEN] |= attacks;
            attacksByColor[PieceColor::WHITE] |= attacks;
            combinedAttacks |= attacks;
            attacksFrom[square] |= attacks;
        }

        uint64_t whiteKing = bitboard.getPieceBoard(PieceType::WHITE_KING);
        while (whiteKing) {
            int square = popLsb(whiteKing);
            uint64_t attacks = bitboard.getKingAttacks(square);

            attacksByPiece[PieceType::WHITE_KING] |= attacks;
            attacksByColor[PieceColor::WHITE] |= attacks;
            combinedAttacks |= attacks;
            attacksFrom[square] |= attacks;
        }

        uint64_t blackPawns = bitboard.getPieceBoard(PieceType::BLACK_PAWN);
        while (blackPawns) {
            int square = popLsb(blackPawns);
            uint64_t attacks = bitboard.getPawnAttacks<PieceColor::BLACK>(square);

            attacksByPiece[PieceType::BLACK_PAWN] |= attacks;
            attacksByColor[PieceColor::BLACK] |= attacks;
            combinedAttacks |= attacks;
            attacksFrom[square] |= attacks;
        }

        uint64_t blackKnights = bitboard.getPieceBoard(PieceType::BLACK_KNIGHT);
        while (blackKnights) {
            int square = popLsb(blackKnights);
            uint64_t attacks = bitboard.getKnightAttacks(square);

            attacksByPiece[PieceType::BLACK_KNIGHT] |= attacks;
            attacksByColor[PieceColor::BLACK] |= attacks;
            combinedAttacks |= attacks;
            attacksFrom[square] |= attacks;
        }

        uint64_t blackBishops = bitboard.getPieceBoard(PieceType::BLACK_BISHOP);
        while (blackBishops) {
            int square = popLsb(blackBishops);
            uint64_t attacks = bitboard.getBishopAttacks(square);

            attacksByPiece[PieceType::BLACK_BISHOP] |= attacks;
            attacksByColor[PieceColor::BLACK] |= attacks;
            combinedAttacks |= attacks;
            attacksFrom[square] |= attacks;
        }

        uint64_t blackRooks = bitboard.getPieceBoard(PieceType::BLACK_ROOK);
        while (blackRooks) {
            int square = popLsb(blackRooks);
            uint64_t attacks = bitboard.getRookAttacks(square);

            attacksByPiece[PieceType::BLACK_ROOK] |= attacks;
            attacksByColor[PieceColor::BLACK] |= attacks;
            combinedAttacks |= attacks;
            attacksFrom[square] |= attacks;
        }

        uint64_t blackQueens = bitboard.getPieceBoard(PieceType::BLACK_QUEEN);
        while (blackQueens) {
            int square = popLsb(blackQueens);
            uint64_t attacks = bitboard.getQueenAttacks(square);

            attacksByPiece[PieceType::BLACK_QUEEN] |= attacks;
            attacksByColor[PieceColor::BLACK] |= attacks;
            combinedAttacks |= attacks;
            attacksFrom[square] |= attacks;
        }

        uint64_t blackKing = bitboard.getPieceBoard(PieceType::BLACK_KING);
        while (blackKing) {
            int square = popLsb(blackKing);
            uint64_t attacks = bitboard.getKingAttacks(square);

            attacksByPiece[PieceType::BLACK_KING] |= attacks;
            attacksByColor[PieceColor::BLACK] |= attacks;
            combinedAttacks |= attacks;
            attacksFrom[square] |= attacks;
        }
    }

    constexpr int knightPhase = 1;
    constexpr int bishopPhase = 1;
    constexpr int rookPhase = 2;
    constexpr int queenPhase = 4;
    constexpr int totalPhase = knightPhase * 4 + bishopPhase * 4 + rookPhase * 4 + queenPhase * 2;

    int Evaluation::getPhase() {
        int phase = totalPhase;

        phase -= popcnt(bitboard.getPieceBoard(PieceType::WHITE_KNIGHT) | bitboard.getPieceBoard(PieceType::BLACK_KNIGHT)) * knightPhase;
        phase -= popcnt(bitboard.getPieceBoard(PieceType::WHITE_BISHOP) | bitboard.getPieceBoard(PieceType::BLACK_BISHOP)) * bishopPhase;
        phase -= popcnt(bitboard.getPieceBoard(PieceType::WHITE_ROOK) | bitboard.getPieceBoard(PieceType::BLACK_ROOK)) * rookPhase;
        phase -= popcnt(bitboard.getPieceBoard(PieceType::WHITE_QUEEN) | bitboard.getPieceBoard(PieceType::BLACK_QUEEN)) * queenPhase;

        return (phase * 256 + (totalPhase / 2)) / totalPhase;
    }

    bool isNotPawnOrKing(PieceType pieceType) {
        return pieceType != PieceType::WHITE_PAWN && pieceType != PieceType::BLACK_PAWN && pieceType != PieceType::WHITE_KING && pieceType != PieceType::BLACK_KING;
    }

    bool isKing(PieceType pieceType) {
        return pieceType == PieceType::WHITE_KING || pieceType == PieceType::BLACK_KING;
    }

    void Evaluation::addMobilityScoreForPiece(PieceType pieceType, int mobility) {
        switch (pieceType) {
            case PieceType::WHITE_KNIGHT:
                whiteMidgameScore += mobility * getEvalValue(MIDGAME_KNIGHT_MOBILITY);
                whiteEndgameScore += mobility * getEvalValue(ENDGAME_KNIGHT_MOBILITY);
                break;
            case PieceType::BLACK_KNIGHT:
                blackMidgameScore += mobility * getEvalValue(MIDGAME_KNIGHT_MOBILITY);
                blackEndgameScore += mobility * getEvalValue(ENDGAME_KNIGHT_MOBILITY);
                break;
            case PieceType::WHITE_BISHOP:
                whiteMidgameScore += mobility * getEvalValue(MIDGAME_BISHOP_MOBILITY);
                whiteEndgameScore += mobility * getEvalValue(ENDGAME_BISHOP_MOBILITY);
                break;
            case PieceType::BLACK_BISHOP:
                blackMidgameScore += mobility * getEvalValue(MIDGAME_BISHOP_MOBILITY);
                blackEndgameScore += mobility * getEvalValue(ENDGAME_BISHOP_MOBILITY);
                break;
            case PieceType::WHITE_ROOK:
                whiteMidgameScore += mobility * getEvalValue(MIDGAME_ROOK_MOBILITY);
                whiteEndgameScore += mobility * getEvalValue(ENDGAME_ROOK_MOBILITY);
                break;
            case PieceType::BLACK_ROOK:
                blackMidgameScore += mobility * getEvalValue(MIDGAME_ROOK_MOBILITY);
                blackEndgameScore += mobility * getEvalValue(ENDGAME_ROOK_MOBILITY);
                break;
            case PieceType::WHITE_QUEEN:
                whiteMidgameScore += mobility * getEvalValue(MIDGAME_QUEEN_MOBILITY);
                whiteEndgameScore += mobility * getEvalValue(ENDGAME_QUEEN_MOBILITY);
                break;
            case PieceType::BLACK_QUEEN:
                blackMidgameScore += mobility * getEvalValue(MIDGAME_QUEEN_MOBILITY);
                blackEndgameScore += mobility * getEvalValue(ENDGAME_QUEEN_MOBILITY);
                break;
        }
    }

    template<PieceColor color>
    void Evaluation::evaluatePieces() {
        uint64_t colorBoard = bitboard.getColorBoard<color>();

        while (colorBoard) {
            uint8_t index = popLsb(colorBoard);
            PieceType pieceType = bitboard.getPieceOnSquare(index);

            // Mobility
            if (isNotPawnOrKing(pieceType)) {
                uint64_t mobilitySquares = attacksFrom[index];

                if (color == PieceColor::WHITE) {
                    // Exclude own pieces and attacks by opponent pawns
                    mobilitySquares &= ~(bitboard.getColorBoard<PieceColor::WHITE>() | attacksByPiece[PieceType::BLACK_PAWN]);

                    // If pieceType == queen, exclude tiles attacked by opponent bishop, knight and rook
                    if (pieceType == PieceType::WHITE_QUEEN) {
                        mobilitySquares &= ~(attacksByPiece[PieceType::BLACK_BISHOP] | attacksByPiece[PieceType::BLACK_KNIGHT] | attacksByPiece[PieceType::BLACK_ROOK]);
                    }

                    // If pieceType == rook, exclude tiles attacked by opponent bishop and knight
                    if (pieceType == PieceType::WHITE_ROOK) {
                        mobilitySquares &= ~(attacksByPiece[PieceType::BLACK_BISHOP] | attacksByPiece[PieceType::BLACK_KNIGHT]);
                    }
                } else {
                    // Exclude own pieces and attacks by opponent pawns
                    mobilitySquares &= ~(bitboard.getColorBoard<PieceColor::BLACK>() | attacksByPiece[PieceType::WHITE_PAWN]);

                    // If pieceType == queen, exclude tiles attacked by opponent bishop, knight and rook
                    if (pieceType == PieceType::BLACK_QUEEN) {
                        mobilitySquares &= ~(attacksByPiece[PieceType::WHITE_BISHOP] | attacksByPiece[PieceType::WHITE_KNIGHT] | attacksByPiece[PieceType::WHITE_ROOK]);
                    }

                    // If pieceType == rook, exclude tiles attacked by opponent bishop and knight
                    if (pieceType == PieceType::BLACK_ROOK) {
                        mobilitySquares &= ~(attacksByPiece[PieceType::WHITE_BISHOP] | attacksByPiece[PieceType::WHITE_KNIGHT]);
                    }
                }

                uint8_t mobility = popcnt(mobilitySquares);
                addMobilityScoreForPiece(pieceType, mobility);
            }

            // King safety
            if (isKing(pieceType)) {
                if (color == PieceColor::WHITE) {
                    // Pawn Shield
                    uint64_t kingBB = bitboard.getPieceBoard(PieceType::WHITE_KING);
                    uint64_t pawnBB = bitboard.getPieceBoard(PieceType::WHITE_PAWN);
                    uint64_t pawnShieldMask = nortOne(kingBB) | noEaOne(kingBB) | noWeOne(kingBB);
                    pawnShieldMask |= nortOne(pawnShieldMask);
                    uint64_t pawnShield = pawnBB & pawnShieldMask;
                    uint8_t pawnShieldCount = std::min(popcnt(pawnShield), 3ULL);

                    whiteMidgameScore += getEvalValue(MIDGAME_PAWN_SHIELD) * pawnShieldCount;
                    whiteEndgameScore += getEvalValue(ENDGAME_PAWN_SHIELD) * pawnShieldCount;

                    // Virtual mobility - Get queen attacks from king position, with only occupied squares by own pieces. We also ignore the squares around the king.
                    uint64_t virtualMobilitySquares = bitboard.getQueenAttacks(index, bitboard.getColorBoard<PieceColor::WHITE>()) & ~(attacksFrom[index] | bitboard.getColorBoard<PieceColor::WHITE>());
                    whiteMidgameScore += popcnt(virtualMobilitySquares) * getEvalValue(MIDGAME_KING_VIRTUAL_MOBILITY_PENALTY);
                    whiteEndgameScore += popcnt(virtualMobilitySquares) * getEvalValue(ENDGAME_KING_VIRTUAL_MOBILITY_PENALTY);
                } else {
                    // Pawn Shield
                    uint64_t kingBB = bitboard.getPieceBoard(PieceType::BLACK_KING);
                    uint64_t pawnBB = bitboard.getPieceBoard(PieceType::BLACK_PAWN);
                    uint64_t pawnShieldMask = soutOne(kingBB) | soEaOne(kingBB) | soWeOne(kingBB);
                    pawnShieldMask |= soutOne(pawnShieldMask);
                    uint64_t pawnShield = pawnBB & pawnShieldMask;
                    uint8_t pawnShieldCount = std::min(popcnt(pawnShield), 3ULL);

                    blackMidgameScore += getEvalValue(MIDGAME_PAWN_SHIELD) * pawnShieldCount;
                    blackEndgameScore += getEvalValue(ENDGAME_PAWN_SHIELD) * pawnShieldCount;

                    // Virtual mobility - Get queen attacks from king position, with only occupied squares by own pieces. We also ignore the squares around the king.
                    uint64_t virtualMobilitySquares = bitboard.getQueenAttacks(index, bitboard.getColorBoard<PieceColor::BLACK>()) & ~(attacksFrom[index] | bitboard.getColorBoard<PieceColor::BLACK>());
                    blackMidgameScore += popcnt(virtualMobilitySquares) * getEvalValue(MIDGAME_KING_VIRTUAL_MOBILITY_PENALTY);
                    blackEndgameScore += popcnt(virtualMobilitySquares) * getEvalValue(ENDGAME_KING_VIRTUAL_MOBILITY_PENALTY);
                }
            }
        }
    }

    int Evaluation::evaluate() {
        int phase = getPhase();
        int modifier = bitboard.getMovingColor() == PieceColor::WHITE ? 1 : -1;

        evaluateMaterial<PieceColor::WHITE>();
        evaluateMaterial<PieceColor::BLACK>();

        evaluatePst<PieceColor::WHITE>();
        evaluatePst<PieceColor::BLACK>();

        evaluatePieces<PieceColor::WHITE>();
        evaluatePieces<PieceColor::BLACK>();

        int whiteScore = ((whiteMidgameScore * (256 - phase)) + (whiteEndgameScore * phase)) / 256;
        int blackScore = ((blackMidgameScore * (256 - phase)) + (blackEndgameScore * phase)) / 256;

        return (whiteScore - blackScore) * modifier;
    }

    template<PieceColor color>
    void Evaluation::evaluateMaterial() {
        if (color == PieceColor::WHITE) {
            whiteMidgameScore += bitboard.getMaterialCount<WHITE_PAWN>() * getEvalValue(MIDGAME_PAWN_MATERIAL);
            whiteEndgameScore += bitboard.getMaterialCount<WHITE_PAWN>() * getEvalValue(ENDGAME_PAWN_MATERIAL);

            whiteMidgameScore += bitboard.getMaterialCount<WHITE_KNIGHT>() * getEvalValue(MIDGAME_KNIGHT_MATERIAL);
            whiteEndgameScore += bitboard.getMaterialCount<WHITE_KNIGHT>() * getEvalValue(ENDGAME_KNIGHT_MATERIAL);

            whiteMidgameScore += bitboard.getMaterialCount<WHITE_BISHOP>() * getEvalValue(MIDGAME_BISHOP_MATERIAL);
            whiteEndgameScore += bitboard.getMaterialCount<WHITE_BISHOP>() * getEvalValue(ENDGAME_BISHOP_MATERIAL);

            whiteMidgameScore += bitboard.getMaterialCount<WHITE_ROOK>() * getEvalValue(MIDGAME_ROOK_MATERIAL);
            whiteEndgameScore += bitboard.getMaterialCount<WHITE_ROOK>() * getEvalValue(ENDGAME_ROOK_MATERIAL);

            whiteMidgameScore += bitboard.getMaterialCount<WHITE_QUEEN>() * getEvalValue(MIDGAME_QUEEN_MATERIAL);
            whiteEndgameScore += bitboard.getMaterialCount<WHITE_QUEEN>() * getEvalValue(ENDGAME_QUEEN_MATERIAL);
        } else {
            blackMidgameScore += bitboard.getMaterialCount<BLACK_PAWN>() * getEvalValue(MIDGAME_PAWN_MATERIAL);
            blackEndgameScore += bitboard.getMaterialCount<BLACK_PAWN>() * getEvalValue(ENDGAME_PAWN_MATERIAL);

            blackMidgameScore += bitboard.getMaterialCount<BLACK_KNIGHT>() * getEvalValue(MIDGAME_KNIGHT_MATERIAL);
            blackEndgameScore += bitboard.getMaterialCount<BLACK_KNIGHT>() * getEvalValue(ENDGAME_KNIGHT_MATERIAL);

            blackMidgameScore += bitboard.getMaterialCount<BLACK_BISHOP>() * getEvalValue(MIDGAME_BISHOP_MATERIAL);
            blackEndgameScore += bitboard.getMaterialCount<BLACK_BISHOP>() * getEvalValue(ENDGAME_BISHOP_MATERIAL);

            blackMidgameScore += bitboard.getMaterialCount<BLACK_ROOK>() * getEvalValue(MIDGAME_ROOK_MATERIAL);
            blackEndgameScore += bitboard.getMaterialCount<BLACK_ROOK>() * getEvalValue(ENDGAME_ROOK_MATERIAL);

            blackMidgameScore += bitboard.getMaterialCount<BLACK_QUEEN>() * getEvalValue(MIDGAME_QUEEN_MATERIAL);
            blackEndgameScore += bitboard.getMaterialCount<BLACK_QUEEN>() * getEvalValue(ENDGAME_QUEEN_MATERIAL);
        }
    }

    template<PieceColor color>
    void Evaluation::evaluatePst() {
        if (color == PieceColor::WHITE) {
            whiteMidgameScore += bitboard.getWhiteMidgamePst();
            whiteEndgameScore += bitboard.getWhiteEndgamePst();
        } else {
            blackMidgameScore += bitboard.getBlackMidgamePst();
            blackEndgameScore += bitboard.getBlackEndgamePst();
        }
    }
}