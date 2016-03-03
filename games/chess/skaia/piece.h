#pragma once

// This class is for intermediate calculations using pieces

#include "common.h"

namespace Skaia
{
    class Piece
    {
        public:
            Position pos;
            Type type;
            Color color; // 0=white, 1=black

            Piece(size_t rank, size_t file, Type type, Color color);
            Piece(const Position& pos, Type type, Color color);
    }
}

