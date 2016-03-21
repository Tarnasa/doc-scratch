#pragma once
// Minimax and its many variations

#include "SkaiaState.h"

namespace Skaia
{
    struct MMReturn
    {
        int heuristic;
        Action action;
        bool checkmate;

    };

    // looks depth_remaining ply deep from the given state and returns
    //  the best heuristic and move that leads there.
    // Min/Max player is a function of .turn variable in state.
    MMReturn minimax(const State& state, Color me, int depth_remaining);

    // Material + bonus for checking the king
    int material(const State& state, Color me, bool stalemate, bool draw);
}

