#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <iostream>

namespace Skaia
{
    enum Type : int {Empty, Pawn, Bishop, Knight, Rook, Queen, King, NumberOfTypes};
    enum TypeMask : uint8_t
    {
        MEmpty =  0,
        MPawn =   1 << 0,
        MBishop = 1 << 1,
        MKnight = 1 << 2,
        MRook =   1 << 3,
        MQueen =  1 << 4,
        MKing =   1 << 5
    };
    enum Color : bool {White, Black};
    static const std::vector<std::string> names = {"Empty", "Pawn", "Bishop", "Knight", "Rook", "Queen", "King"};

    int file_to_skaia(const std::string& file);
    std::string file_from_skaia(int file);
    int rank_to_skaia(int rank);
    int rank_from_skaia(int rank);
    Type type_to_skaia(const std::string& type);
    std::string type_from_skaia(Type type);

    template<typename T> T sign(const T& x) { return (x > 0) - (x < 0); }

    struct Position
    {
        int rank, file;
        Position() {}
        Position(int rank, int file) : rank(rank), file(file) {}
        Position(const Position& source) = default;
        Position(Position&& source) = default;
        Position& operator=(const Position& source) = default;
        Position& operator=(Position&& source) = default;
        bool operator==(const Position& rhs) const { return rank == rhs.rank && file == rhs.file; }
        bool operator!=(const Position& rhs) const { return rank != rhs.rank || file != rhs.file; }
        Position& operator+=(const Position& rhs) { rank += rhs.rank; file += rhs.file; }
        Position& operator-=(const Position& rhs) { rank -= rhs.rank; file -= rhs.file; }
        Position& operator*=(int factor) { rank *= factor; file *= factor; }
        Position operator+(const Position& rhs) const
        {
            Position new_pos(*this);
            new_pos += rhs;
            return new_pos;
        }
        Position operator-(const Position& rhs) const
        {
            Position new_pos(*this);
            new_pos -= rhs;
            return new_pos;
        }
        Position operator*(int factor) const
        {
            Position new_pos(*this);
            new_pos *= factor;
            return new_pos;
        }
        Position direction_to(const Position& to) const
        {
            Position delta = to - *this;
            delta.rank = sign(delta.rank);
            delta.file = sign(delta.file);
            return delta;
        }
    };
}

std::ostream& operator<<(std::ostream& out, const Skaia::Position& pos);

