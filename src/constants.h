
/*
 This file is part of Zagreus.

 Zagreus is a UCI chess engine
 Copyright (C) 2023-2025  Danny Jelsma

 Zagreus is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published
 by the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Zagreus is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with Zagreus.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstdint>

#define NOT_A_FILE 0xFEFEFEFEFEFEFEFEULL
#define NOT_AB_FILE 0xFCFCFCFCFCFCFCFCULL
#define NOT_GH_FILE 0x3F3F3F3F3F3F3F3FULL
#define NOT_H_FILE 0x7F7F7F7F7F7F7F7FULL

#define RANK_1 0x00000000000000FFULL
#define RANK_2 0x000000000000FF00ULL
#define RANK_3 0x0000000000FF0000ULL
#define RANK_4 0x00000000FF000000ULL
#define RANK_5 0x000000FF00000000ULL
#define RANK_6 0x0000FF0000000000ULL
#define RANK_7 0x00FF000000000000ULL
#define RANK_8 0xFF00000000000000ULL

#define LIGHT_SQUARES 0x55AA55AA55AA55AAULL
#define DARK_SQUARES 0xAA55AA55AA55AA55ULL
#define CENTER_SQUARES 0x0000001818000000ULL
#define EXTENDED_CENTER_SQUARES 0x00003C3C3C3C0000ULL
#define PROMOTION_SQUARES 0xFF000000000000FFULL

#define WHITE_KINGSIDE_CASTLE_PATH 0x60ULL
#define WHITE_QUEENSIDE_CASTLE_PATH 0xCULL
#define BLACK_KINGSIDE_CASTLE_PATH 0x6000000000000000ULL
#define BLACK_QUEENSIDE_CASTLE_PATH 0xC00000000000000ULL

#define WHITE_KINGSIDE_CASTLE_UNOCCUPIED 0x60ULL
#define WHITE_QUEENSIDE_CASTLE_UNOCCUPIED 0xEULL
#define BLACK_KINGSIDE_CASTLE_UNOCCUPIED 0x6000000000000000ULL
#define BLACK_QUEENSIDE_CASTLE_UNOCCUPIED 0xE00000000000000ULL

#define SQUARES 64
#define PIECES 12
#define PIECE_TYPES 6
#define COLORS 2
#define GAME_PHASES 2
#define MIDGAME 0
#define ENDGAME 1

#define MAX_PLIES 750
#define MAX_MOVES 255
#define NO_MOVE 0

#define NO_TT_SCORE INT16_MIN

#define MAX_HISTORY 16384

#define DRAW_SCORE 0
#define MATE_SCORE 25000
#define INITIAL_ALPHA (-32000)
#define INITIAL_BETA 32000

#define ZOBRIST_PIECE_START_INDEX 0
#define ZOBRIST_SIDE_TO_MOVE_INDEX PIECES * SQUARES
#define ZOBRIST_CASTLING_WHITE_KINGSIDE_INDEX ZOBRIST_SIDE_TO_MOVE_INDEX + 1
#define ZOBRIST_CASTLING_WHITE_QUEENSIDE_INDEX ZOBRIST_CASTLING_WHITE_KINGSIDE_INDEX + 1
#define ZOBRIST_CASTLING_BLACK_KINGSIDE_INDEX ZOBRIST_CASTLING_WHITE_QUEENSIDE_INDEX + 1
#define ZOBRIST_CASTLING_BLACK_QUEENSIDE_INDEX ZOBRIST_CASTLING_BLACK_KINGSIDE_INDEX + 1
#define ZOBRIST_EN_PASSANT_START_INDEX ZOBRIST_CASTLING_BLACK_QUEENSIDE_INDEX + 1
