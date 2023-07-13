/*
 This file is part of Zagreus.

 Zagreus is a chess engine that supports the UCI protocol
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

#include <chrono>
#include <algorithm>
#include <iostream>

#include "../senjo/GoParams.h"
#include "types.h"
#include "engine.h"

namespace Zagreus {
    std::chrono::time_point<std::chrono::steady_clock> getEndTime(std::chrono::time_point<std::chrono::steady_clock> startTime, senjo::GoParams &params, Bitboard &bitboard, ZagreusEngine &engine, PieceColor movingColor) {
        if (params.infinite || params.depth > 0 || params.nodes > 0) {
            return std::chrono::time_point<std::chrono::steady_clock>::max();
        }

        if (params.movetime > 0) {
            return startTime + std::chrono::milliseconds(params.movetime - engine.getOption("MoveOverhead").getIntValue());
        }

        int movesToGo = params.movestogo ? params.movestogo : 50ULL;

//        int movesToGo = moves - (bitboard.getPly() / 2);

        uint64_t timeLeft = 0;

        if (movingColor == PieceColor::WHITE) {
            timeLeft += params.wtime;
            timeLeft += params.winc * movesToGo;
        } else {
            timeLeft += params.btime;
            timeLeft += params.binc * movesToGo;
        }

        uint64_t moveOverhead = engine.getOption("MoveOverhead").getIntValue();
        timeLeft -= moveOverhead * movesToGo;

        timeLeft = std::max((uint64_t) timeLeft, (uint64_t) 1ULL);
        uint64_t maxTime;

        if (movingColor == PieceColor::WHITE) {
            if (params.wtime > moveOverhead) {
                maxTime = (params.wtime - moveOverhead) / 100 * 80;
            } else {
                maxTime = (params.wtime / 2) / 100 * 80;
            }
        } else {
            if (params.btime > moveOverhead) {
                maxTime = (params.btime - moveOverhead) / 100 * 80;
            } else {
                maxTime = (params.btime / 2) / 100 * 80;
            }
        }

        uint64_t timePerMove = timeLeft / movesToGo;

        if (timePerMove > maxTime) {
            timePerMove = maxTime;
        }

        timePerMove = std::max((uint64_t) timePerMove, (uint64_t) 1ULL);
        return startTime + std::chrono::milliseconds(timePerMove);
    }
}
