//
// Created by Danny on 28-4-2022.
//

#include <chrono>
#include "time_mgr.h"
#include "bitboard.h"

namespace Zagreus {
    std::chrono::time_point<std::chrono::high_resolution_clock>
    TimeManager::getEndTime(Bitboard &bitboard, PieceColor movingColor) {
        int movesLeft = 80 - bitboard.getMovesMade();
        uint64_t timeLeft =
                movingColor == PieceColor::WHITE ? bitboard.getWhiteTimeMsec() : bitboard.getBlackTimeMsec();

        if (movesLeft < 6) {
            movesLeft = 6;
        }

        if (timeLeft == 0) {
            timeLeft = 300000;
        }

        uint64_t timePerMove = timeLeft / movesLeft;

        std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
        now += std::chrono::milliseconds(timePerMove);
        return now;
    }
}
