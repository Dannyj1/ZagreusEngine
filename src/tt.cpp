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

#include <iostream>
#include <cmath>

#include "tt.h"

namespace Zagreus {
    void TranspositionTable::addPosition(uint64_t zobristHash, int depth, int score, NodeType nodeType) {
        if (score >= 90000000 || score <= -90000000) {
            return;
        }

        uint64_t index = (zobristHash & hashSize);
        TTEntry* entry = &transpositionTable[index];

        if (entry->depth <= depth) {
            entry->zobristHash = zobristHash;
            entry->depth = depth;
            entry->score = score;
            entry->nodeType = nodeType;
        }
    }

    int TranspositionTable::getScore(uint64_t zobristHash, int depth, int alpha, int beta) {
        uint64_t index = (zobristHash & hashSize);
        TTEntry* entry = &transpositionTable[index];

        if (entry->zobristHash == zobristHash && entry->depth >= depth) {
            if (entry->nodeType == PV_NODE) {
                return entry->score;
            } else if (entry->nodeType == FAIL_LOW_NODE) {
                if (entry->score <= alpha) {
                    return alpha;
                }
            } else if (entry->nodeType == FAIL_HIGH_NODE) {
                if (entry->score >= beta) {
                    return beta;
                }
            }
        }

        return INT32_MIN;
    }

    TTEntry* TranspositionTable::getEntry(uint64_t zobristHash) {
        uint64_t index = (zobristHash & hashSize);

        return &transpositionTable[index];
    }

    void TranspositionTable::setTableSize(int megaBytes) {
        if ((megaBytes & (megaBytes - 1)) != 0) {
            megaBytes = 1 << (int) (log2(megaBytes));
        }

        uint64_t byteSize = megaBytes * 1024 * 1024;
        uint64_t entryCount = byteSize / sizeof(TTEntry);

        delete[] transpositionTable;
        transpositionTable = new TTEntry[entryCount]{};
        hashSize = entryCount - 1;
        
        for (uint64_t i = 0; i < entryCount; i++) {
            transpositionTable[i] = {};
        }
    }

    TranspositionTable* TranspositionTable::TranspositionTable::getTT() {
        static TranspositionTable instance{};
        return &instance;
    }

    void TranspositionTable::ageHistoryTable() {
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 64; j++) {
                historyMoves[i][j] /= 8;
            }
        }
    }
}
