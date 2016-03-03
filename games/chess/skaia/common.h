#pragma once

#include <vector>
#include <cstdint>

namespace Skaia
{
    enum Type : uint8_t {Empty, Pawn, Bishop, Knight, Rook, Queen, King};
    enum TypeMask : uint8_t
    {
        Empty =  0;
        Pawn =   1 << 0;
        Bishop = 1 << 1;
        Knight = 1 << 2;
        Rook =   1 << 3;
        Queen =  1 << 4;
        King =   1 << 5;
    };
    enum Colot : bool {Black, White};
    static const std::vector<std::string> names = {"Pawn", "Bishop", "Knight", "Rook", "Queen", "King"};

    struct Position
    {
        size_t rank, file;
        Position() {}
        Position(size_t rank, size_t file) : rank(rank), file(file) {}
        Position(const Position& source) = default;
        Position(Position&& source) = default;
        Position& operator=(const Position& source) = default;
        Position& operator=(Position&& source) = default;
        bool Position==(const Position& rhs) { return rank == rhs.rank && file == rhs.file; }
        bool Position!=(const Position& rhs) { return rank != rhs.rank || file != rhs.file; }
        Position& operator+=(const Position& rhs) { rank += rhs.rank; file += rhs.file; }
        Position operator+(const Position& lhs, const Position& rhs) { Position new_pos(lhs); return lhs += rhs; }
    };

    other_color(Color color) { return !color; }
}

