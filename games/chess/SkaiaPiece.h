#pragma once

// This class is for intermediate calculations using pieces

#include "Skaia.h"

#include "SkaiaAction.h"

namespace Skaia
{
    class Piece
    {
        public:
            Position pos;
            Type type;
            Color color; // 0=white, 1=black
            int id;
            bool alive; // Whether or not this piece is still a part of the game
            std::vector<Action> moves; // A vector of all possible moves this piece can make
            int special; // True if this piece has never moved

            // For pawns, bishops, rooks, kings, and queens, this variable tells how far the piece
            //  can move in each of the 8 directions starting with (-1, 0), then (-1, 1).
            // For knights, the order of directions is: (-2, 1), (-1, 2), ...
            //std::array<int, 8> move_distance;

            Piece() : type(Empty), id(-1), special(true), alive(true) {}
            Piece(const Position& pos, Type type, Color color, size_t id, bool alive, size_t special = 0) :
                pos(pos), type(type), color(color), id(id), alive(alive), special(special) {}
            Piece(const Piece& source) = default;
            Piece& operator=(const Piece& rhs) = default;
    };
}

