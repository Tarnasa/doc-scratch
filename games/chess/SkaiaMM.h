#pragma once
// Minimax and its many variations

#include "SkaiaState.h"
#include "HistoryTable.h"

#include <map>
#include <atomic>

namespace Skaia
{
    struct MMReturn
    {
        int heuristic;
        Action action;
        int states_evaluated;
    };

    // looks depth_remaining ply deep from the given state and returns
    //  the best heuristic and move that leads there.
    // Min/Max player is a function of .turn variable in state.
    MMReturn minimax(const State& cstate, Color me, int depth_remaining, int lower, int upper);

    // Same as minimax, but stops trying new actions when &stop is true
    MMReturn interruptable_minimax(const State& cstate, Color me, int depth_remaining,
            int lower, int upper, std::atomic<bool> &stop);

    // Like minimax(), but does no pruning on the top level, and returns the best action found for each top-level action
    std::vector<std::pair<Action, MMReturn>> pondering_minimax(const State& cstate, Color me,
            int depth_remaining, int lower, int upper, std::atomic<bool> &stop);

    // Material + net checks
    int heuristic(const State& state, Color me, bool stalemate, bool draw);

    // 
}

