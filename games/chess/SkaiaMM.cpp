#include "SkaiaMM.h"

#include <limits>
#include <iostream>

namespace Skaia
{
    MMReturn minimax(const State& state, Color me, int depth_remaining)
    {
        static const Action empty_action(Position(-1, -1), Position(-1, -1), Empty);
        auto moves = state.generate_actions();
        bool stalemate = moves.empty();
        bool draw = state.draw();
        if (stalemate || draw || depth_remaining == 0)
        {
            return MMReturn{material(state, me, stalemate, draw), empty_action};
        }
        else if ((state.turn % 2 ? Black : White) == me)
        {
            // Maximizing
            MMReturn best = MMReturn{std::numeric_limits<int>::lowest(), empty_action};
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
            MMReturn worst = MMReturn{std::numeric_limits<int>::max(), empty_action};
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

    int material(const State& state, Color me, bool stalemate, bool draw)
    {
        Color current = state.turn % 2 ? Black : White;
        int h = 0;
        // Account for material
        h += state.material(me) - state.material(!me ? Black : White);

        // Add dominating bonus for checkmate
        if (stalemate)
        {
            if (state.is_in_check(current)) // Checkmate
            {
                h += (current == me) ? -1000 : 1000;
            }
            else
            {
                draw = true;
            }
        }

        // Only the losing player wants a draw
        if (draw)
        {
            h = 0;
        }
        return h;
    }
}
