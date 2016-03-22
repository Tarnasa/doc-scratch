#pragma once

// This class is for intermediate calculations using pieces

#include "Skaia.h"

#include "SkaiaAction.h"

#include <set>

namespace Skaia
{
    class Piece
    {
        public:
            Position pos;
            Type type;
            Color color; // 0=white, 1=black
            int id; // This will be an index into the vector of pieces
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

            // For testing purposes
            bool operator==(const Piece& rhs) const
            {
                auto check = [](bool cond, const std::string& message) {
                    if (!cond)
                    {
                        std::cerr << "Assertion: " << message << " failed" << std::endl;
                    }
                    return cond;
                };
                std::set<Action> lhs_set(moves.begin(), moves.end()), rhs_set(rhs.moves.begin(), rhs.moves.end());
                return
                    check(pos == rhs.pos, "piece pos") &&
                    check(type == rhs.type, "piece type") &&
                    check(color == rhs.color, "piece color") &&
                    check(id == rhs.id, "piece id") &&
                    check(alive == rhs.alive, "piece alive") &&
                    check(lhs_set == rhs_set, "piece moves") &&
                    check(special == rhs.special, "piece special");
            }
    };
}

