#include "SkaiaMM.h"

#include <limits>
#include <iostream>

namespace Skaia
{
    MMReturn minimax(const State& cstate, Color me, int depth_remaining, int lower, int upper)
    {
        LOG("minimax");
        // Cast away const-ness (it's ok, back_actions SHOULD return it to the original state)
        State& state = const_cast<State&>(cstate);
        
        static const Action empty_action(Position(-1, -1), Position(-1, -1), Empty);

        auto moves = state.generate_actions();
        bool stalemate = moves.empty();
        bool draw = state.draw();
        // Base case, terminal node
        if (stalemate || draw || depth_remaining == 0)
        {
            LOG("minimax: base case");
            return MMReturn{heuristic(state, me, stalemate, draw), empty_action, 1};
        }
        else if ((state.turn % 2 ? Black : White) == me)
        {
            // Maximizing
            MMReturn best = MMReturn{std::numeric_limits<int>::lowest(), empty_action};
            for (auto& action : moves)
            {
                LOG("minimax: maximizing: " << action);
                auto back_action = state.apply_action(action);
                auto ret = minimax(state, me, depth_remaining - 1, lower, upper);
                state.apply_back_action(back_action);

                best.states_evaluated += ret.states_evaluated;
                if (ret.heuristic > upper)
                {
                    LOG("minimax: short circuit");
                    best = ret;
                    best.action = action;
                    break;
                }
                if (ret.heuristic > best.heuristic)
                {
                    LOG("minimax: new best");
                    best = ret;
                    best.action = action;
                    if (ret.heuristic > lower)
                    {
                        lower = ret.heuristic;
                    }
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
                LOG("minimax: minimizing: " << action);
                auto back_action = state.apply_action(action);
                auto ret = minimax(state, me, depth_remaining - 1, lower, upper);
                state.apply_back_action(back_action);

                worst.states_evaluated += ret.states_evaluated;
                if (ret.heuristic < lower)
                {
                    LOG("minimax: short circuit");
                    worst = ret;
                    worst.action = action;
                    break;
                }
                if (ret.heuristic < worst.heuristic)
                {
                    LOG("minimax: new worst");
                    worst = ret;
                    worst.action = action;
                    if (ret.heuristic < upper)
                    {
                        upper = ret.heuristic;
                    }
                }
            }
            return worst;
        }
    }

    int heuristic(const State& state, Color me, bool stalemate, bool draw)
    {
        Color current = state.turn % 2 ? Black : White;
        int h = 0;
        int my_material = state.material(me);
        int their_material = state.material(!me);
        // Account for material
        h += my_material - their_material;
        h *= 1000;
        // Net checks 
        h += state.count_net_checks(me) - state.count_net_checks(!me);
        h += state.count_net_check_values(me) - state.count_net_check_values(!me);

        // Opening
        if (state.turn <= 10)
        {
            // Mobility
            h += (state.count_piece_moves(me) - state.count_piece_moves(!me)) * 5;
            // TODO: Penalize early queen aggression, lest we lose tempo
        }

        // Endgame
        if (their_material <= 3)
        {
            // Promote the pawns!
            h += state.count_pawn_advancement(me) * 2;
            // Force moves
            h += 8;
            h -= state.pieces_by_color_and_type[!me][King][0]->moves.size();
            // Put the king in check
            h += state.is_in_check(!me) ? 5 : 0;
        }

        // Add dominating bonus for checkmate
        if (stalemate)
        {
            if (state.is_in_check(current)) // Checkmate
            {
                h += (current == me) ? -100000 : 100000;
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
