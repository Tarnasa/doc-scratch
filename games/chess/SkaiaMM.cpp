#include "SkaiaMM.h"

#include <limits>
#include <iostream>

namespace Skaia
{
    MMReturn minimax(const State& state, Color me, int depth_remaining)
    {
        static const Action empty_action(Position(-1, -1), Position(-1, -1), Empty);
        auto moves = state.generate_actions();
        bool checkmate = moves.empty();
        if (checkmate || depth_remaining == 0)
        {
            return MMReturn{material(state, me, checkmate), empty_action, checkmate};
        }
        else if ((state.turn % 2 ? Black : White) == me)
        {
            // Maximizing
            MMReturn best = MMReturn{std::numeric_limits<int>::lowest(), empty_action, false};
            for (auto& action : moves)
            {
                State new_state(state);
                // TODO: Make generate_actions return new states as well
                new_state.apply_action(action);
                auto ret = minimax(new_state, me, depth_remaining - 1);
                if (ret.heuristic > best.heuristic)
                {
                    best = ret;
                    best.action = action;
                }
            }
            return best;
        }
        else
        {
            // Minimizing
            MMReturn worst = MMReturn{std::numeric_limits<int>::max(), empty_action, false};
            for (auto& action : moves)
            {
                State new_state(state);
                new_state.apply_action(action);
                auto ret = minimax(new_state, me, depth_remaining - 1);
                if (ret.heuristic < worst.heuristic)
                {
                    worst = ret;
                    worst.action = action;
                }
            }
            return worst;
        }
    }

    int material(const State& state, Color me, bool checkmate)
    {
        Color current = state.turn % 2 ? Black : White;
        int h = 0;
        // Account for material
        h += state.material(me) - state.material(!me ? Black : White);
        // Add dominating bonus for checkmate
        if (checkmate)
        {
            h += (current == me) ? -1000 : 1000;
        }
        // Only the losing player wants a draw
        if (state.draw())
        {
            std::cout << "Draw" << std::endl;
            h = 0;
        }
        return h;
    }
}
