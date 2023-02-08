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

#include "types.h"


namespace Zagreus {
    class MovePicker {
    private:
        MoveList moveList{};
        int searchStartIndex = 0;
    public:
        MovePicker() = default;

        MovePicker(MoveList moves);

        Move getNextMove();

        bool hasNext() const;

        int size();

        int remaining();

        int movesSearched();
    };
}
