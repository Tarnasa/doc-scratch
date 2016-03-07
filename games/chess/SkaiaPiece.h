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
            int id;
            bool alive;
            int special;
            // std::array<int, 8> moves // How far in each direction this piece can move

            Piece() : type(Empty), id(-1), special(true), alive(true) {}
            Piece(const Position& pos, Type type, Color color, size_t id, bool alive, size_t special = 0) :
                pos(pos), type(type), color(color), id(id), alive(alive), special(special) {}
            Piece(const Piece& source) = default;
            Piece& operator=(const Piece& rhs) = default;
    };
}

