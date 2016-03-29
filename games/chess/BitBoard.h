#pragma once

// A wrapper around a 64-bit integer that allows fast operations commonly
//  found in chess.

#include <cstdint>

class BitBoard
{
    public:
        uint64_t data;

        BitBoard();

        bool at(int n) const;
};

