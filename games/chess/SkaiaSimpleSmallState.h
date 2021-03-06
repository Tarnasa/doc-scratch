#pragma once

#include <array>
#include <iostream>
#include <iomanip>

namespace Skaia
{
    // A simple state representation for checking for draws
    struct SimpleSmallState
    {
        // Every 4 is a square for which:
        //  3 bits - Empty,Pawn,Bishop,Knight,Rook,Queen,King
        //  1 bit - color
        std::array<uint64_t, 4> data;
        bool operator==(const SimpleSmallState& rhs) const { return data == rhs.data; }
        bool operator!=(const SimpleSmallState& rhs) const { return data != rhs.data; }
    };
}

std::ostream& operator<<(std::ostream& out, const Skaia::SimpleSmallState &sss);

