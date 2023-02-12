/*
 Zagreus is a chess engine that supports the UCI protocol
 Copyright (C) 2023  Danny Jelsma

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published
 by the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>

namespace Zagreus {
    enum EvalFeature {
        MIDGAME_PAWN_MATERIAL,
        ENDGAME_PAWN_MATERIAL,
        MIDGAME_KNIGHT_MATERIAL,
        ENDGAME_KNIGHT_MATERIAL,
        MIDGAME_BISHOP_MATERIAL,
        ENDGAME_BISHOP_MATERIAL,
        MIDGAME_ROOK_MATERIAL,
        ENDGAME_ROOK_MATERIAL,
        MIDGAME_QUEEN_MATERIAL,
        ENDGAME_QUEEN_MATERIAL,
        MIDGAME_SQUARE_DEFENDED_BY_PAWN,
        ENDGAME_SQUARE_DEFENDED_BY_PAWN,
        MIDGAME_KNIGHT_MOBILITY,
        ENDGAME_KNIGHT_MOBILITY,
        MIDGAME_BISHOP_MOBILITY,
        ENDGAME_BISHOP_MOBILITY,
        MIDGAME_ROOK_MOBILITY,
        ENDGAME_ROOK_MOBILITY,
        MIDGAME_QUEEN_MOBILITY,
        ENDGAME_QUEEN_MOBILITY,
        MIDGAME_KING_ON_OPEN_FILE,
        ENDGAME_KING_ON_OPEN_FILE,
        MIDGAME_KING_NEXT_TO_OPEN_FILE,
        ENDGAME_KING_NEXT_TO_OPEN_FILE,
        MIDGAME_NO_CASTLING_RIGHTS,
        ENDGAME_NO_CASTLING_RIGHTS,
        MIDGAME_QUEENSIDE_CASTLING_PREVENTED,
        ENDGAME_QUEENSIDE_CASTLING_PREVENTED,
        MIDGAME_KINGSIDE_CASTLING_PREVENTED,
        ENDGAME_KINGSIDE_CASTLING_PREVENTED,
        MIDGAME_BISHOP_ATTACK_NEAR_KING,
        ENDGAME_BISHOP_ATTACK_NEAR_KING,
        MIDGAME_ROOK_ATTACK_NEAR_KING,
        ENDGAME_ROOK_ATTACK_NEAR_KING,
        MIDGAME_QUEEN_ATTACK_NEAR_KING,
        ENDGAME_QUEEN_ATTACK_NEAR_KING,
        MIDGAME_PAWN_CONNECTIVITY,
        ENDGAME_PAWN_CONNECTIVITY,
        MIDGAME_KNIGHT_CONNECTIVITY,
        ENDGAME_KNIGHT_CONNECTIVITY,
        MIDGAME_BISHOP_CONNECTIVITY,
        ENDGAME_BISHOP_CONNECTIVITY,
        MIDGAME_ROOK_CONNECTIVITY,
        ENDGAME_ROOK_CONNECTIVITY,
        MIDGAME_QUEEN_CONNECTIVITY,
        ENDGAME_QUEEN_CONNECTIVITY,
        MIDGAME_ROOK_ON_OPEN_FILE,
        ENDGAME_ROOK_ON_OPEN_FILE,
        MIDGAME_ROOK_ON_SEMI_OPEN_FILE,
        ENDGAME_ROOK_ON_SEMI_OPEN_FILE,
        MIDGAME_ROOK_ON_7TH_RANK,
        ENDGAME_ROOK_ON_7TH_RANK,
        MIDGAME_ROOK_ON_QUEEN_FILE,
        ENDGAME_ROOK_ON_QUEEN_FILE,
        MIDGAME_ROOK_LESS_PAWNS_BONUS,
        ENDGAME_ROOK_LESS_PAWNS_BONUS,
        MIDGAME_NO_BISHOP_PAIR,
        ENDGAME_NO_BISHOP_PAIR,
        MIDGAME_BISHOP_FIANCHETTO,
        ENDGAME_BISHOP_FIANCHETTO,
        MIDGAME_PAWN_ON_SAME_FILE,
        ENDGAME_PAWN_ON_SAME_FILE,
        MIDGAME_PASSED_PAWN,
        ENDGAME_PASSED_PAWN,
        MIDGAME_ISOLATED_SEMI_OPEN_PAWN,
        ENDGAME_ISOLATED_SEMI_OPEN_PAWN,
        MIDGAME_ISOLATED_PAWN,
        ENDGAME_ISOLATED_PAWN,
        MIDGAME_PAWN_SEMI_OPEN_FILE,
        ENDGAME_PAWN_SEMI_OPEN_FILE,
        MIDGAME_PAWN_SHIELD,
        ENDGAME_PAWN_SHIELD,
        MIDGAME_BISHOP_OUTPOST,
        ENDGAME_BISHOP_OUTPOST,
        MIDGAME_KNIGHT_OUTPOST,
        ENDGAME_KNIGHT_OUTPOST,
    };

    static std::vector<const char*> evalFeatureNames = {
            "MIDGAME_PAWN_MATERIAL",
            "ENDGAME_PAWN_MATERIAL",
            "MIDGAME_KNIGHT_MATERIAL",
            "ENDGAME_KNIGHT_MATERIAL",
            "MIDGAME_BISHOP_MATERIAL",
            "ENDGAME_BISHOP_MATERIAL",
            "MIDGAME_ROOK_MATERIAL",
            "ENDGAME_ROOK_MATERIAL",
            "MIDGAME_QUEEN_MATERIAL",
            "ENDGAME_QUEEN_MATERIAL",
            "MIDGAME_SQUARE_DEFENDED_BY_PAWN",
            "ENDGAME_SQUARE_DEFENDED_BY_PAWN",
            "MIDGAME_KNIGHT_MOBILITY",
            "ENDGAME_KNIGHT_MOBILITY",
            "MIDGAME_BISHOP_MOBILITY",
            "ENDGAME_BISHOP_MOBILITY",
            "MIDGAME_ROOK_MOBILITY",
            "ENDGAME_ROOK_MOBILITY",
            "MIDGAME_QUEEN_MOBILITY",
            "ENDGAME_QUEEN_MOBILITY",
            "MIDGAME_KING_ON_OPEN_FILE",
            "ENDGAME_KING_ON_OPEN_FILE",
            "MIDGAME_KING_NEXT_TO_OPEN_FILE",
            "ENDGAME_KING_NEXT_TO_OPEN_FILE",
            "MIDGAME_NO_CASTLING_RIGHTS",
            "ENDGAME_NO_CASTLING_RIGHTS",
            "MIDGAME_QUEENSIDE_CASTLING_PREVENTED",
            "ENDGAME_QUEENSIDE_CASTLING_PREVENTED",
            "MIDGAME_KINGSIDE_CASTLING_PREVENTED",
            "ENDGAME_KINGSIDE_CASTLING_PREVENTED",
            "MIDGAME_BISHOP_ATTACK_NEAR_KING",
            "ENDGAME_BISHOP_ATTACK_NEAR_KING",
            "MIDGAME_ROOK_ATTACK_NEAR_KING",
            "ENDGAME_ROOK_ATTACK_NEAR_KING",
            "MIDGAME_QUEEN_ATTACK_NEAR_KING",
            "ENDGAME_QUEEN_ATTACK_NEAR_KING",
            "MIDGAME_PAWN_CONNECTIVITY",
            "ENDGAME_PAWN_CONNECTIVITY",
            "MIDGAME_KNIGHT_CONNECTIVITY",
            "ENDGAME_KNIGHT_CONNECTIVITY",
            "MIDGAME_BISHOP_CONNECTIVITY",
            "ENDGAME_BISHOP_CONNECTIVITY",
            "MIDGAME_ROOK_CONNECTIVITY",
            "ENDGAME_ROOK_CONNECTIVITY",
            "MIDGAME_QUEEN_CONNECTIVITY",
            "ENDGAME_QUEEN_CONNECTIVITY",
            "MIDGAME_ROOK_ON_OPEN_FILE",
            "ENDGAME_ROOK_ON_OPEN_FILE",
            "MIDGAME_ROOK_ON_SEMI_OPEN_FILE",
            "ENDGAME_ROOK_ON_SEMI_OPEN_FILE",
            "MIDGAME_ROOK_ON_7TH_RANK",
            "ENDGAME_ROOK_ON_7TH_RANK",
            "MIDGAME_ROOK_ON_QUEEN_FILE",
            "ENDGAME_ROOK_ON_QUEEN_FILE",
            "MIDGAME_ROOK_LESS_PAWNS_BONUS",
            "ENDGAME_ROOK_LESS_PAWNS_BONUS",
            "MIDGAME_NO_BISHOP_PAIR",
            "ENDGAME_NO_BISHOP_PAIR",
            "MIDGAME_BISHOP_FIANCHETTO",
            "ENDGAME_BISHOP_FIANCHETTO",
            "MIDGAME_PAWN_ON_SAME_FILE",
            "ENDGAME_PAWN_ON_SAME_FILE",
            "MIDGAME_PASSED_PAWN",
            "ENDGAME_PASSED_PAWN",
            "MIDGAME_ISOLATED_SEMI_OPEN_PAWN",
            "ENDGAME_ISOLATED_SEMI_OPEN_PAWN",
            "MIDGAME_ISOLATED_PAWN",
            "ENDGAME_ISOLATED_PAWN",
            "MIDGAME_PAWN_SEMI_OPEN_FILE",
            "ENDGAME_PAWN_SEMI_OPEN_FILE",
            "MIDGAME_PAWN_SHIELD",
            "ENDGAME_PAWN_SHIELD",
            "MIDGAME_BISHOP_OUTPOST",
            "ENDGAME_BISHOP_OUTPOST",
            "MIDGAME_KNIGHT_OUTPOST",
            "ENDGAME_KNIGHT_OUTPOST",
    };

    void printEvalValues();

    int getEvalValue(EvalFeature feature);

    int getEvalFeatureSize();

    std::vector<double> getEvalValues();

    std::vector<double> getBaseEvalValues();

    void updateEvalValues(std::vector<double> &newValues);
}
