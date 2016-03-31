#pragma once

// Mostly a heuristic class for determining pieces how much a piece is
//  protected, and in danger.
// Also used for determining if a king is in check.

#include <array>

class BoardChecks
{
    public:
        std::array<std::pair<int, int>, 8 * 8> checks; // For each square, the heurisitc for white and black

        BoardChecks();

};


