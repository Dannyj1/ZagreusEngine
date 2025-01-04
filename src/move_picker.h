/*
 This file is part of Zagreus.

 Zagreus is a UCI chess engine
 Copyright (C) 2023-2025  Danny Jelsma

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

#include <array>

#include "constants.h"
#include "move.h"
#include "types.h"

namespace Zagreus {
// TODO: Implement efficient sorting by score, after implementing score.
class MovePicker {
private:
    MoveList moveList{};
    int currentIndex = 0;

public:
    explicit MovePicker(const MoveList& moveList) : moveList(moveList) {}

    /**
     * \brief Checks if there is a next move and retrieves it.
     * \param[out] move The next move if available.
     * \return True if there is a next move, false otherwise.
     */
    [[nodiscard]] bool next(Move& move) {
        if (currentIndex >= moveList.size) {
            return false;
        }

        move = moveList.moves[currentIndex];
        currentIndex += 1;
        return true;
    }

    /**
     * \brief Gets the current move.
     * \return The current move.
     */
    [[nodiscard]] Move getCurrentMove() const {
        return moveList.moves[currentIndex - 1];
    }
};
} // namespace Zagreus
