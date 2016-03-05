#pragma once

// This class is for intermediate calculations using pieces

#include "Skaia.h"

namespace Skaia
{
    class Piece
    {
        public:
            Position pos;
            Type type;
            Color color; // 0=white, 1=black
            size_t special;

            Piece(const Position& pos, Type type, Color color, size_t special = 0) pos(pos), type(type), color(color), special(special) {}
    }
}

