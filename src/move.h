
/*
 This file is part of Zagreus.

 Zagreus is a UCI chess engine
 Copyright (C) 2023-2024  Danny Jelsma

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

#pragma once
#include "constants.h"


namespace Zagreus {
using Move = uint16_t;

struct MoveList {
    std::array<Move, MAX_MOVES> list{};
    uint8_t size = 0;
};

// bits 0-5: to square (0-63)
// bits 6-11: from square (0-63)
inline Move encodeMove(const uint8_t fromSquare, const uint8_t toSquare) {
    return (fromSquare << 6) | toSquare;
}
} // namespace Zagreus
