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
        else
        {
            bool maximizing = (state.turn % 2 ? Black : White) == me;
            // Initialize our "best" action with the worst possible action
            auto starting_heuristic = maximizing ? std::numeric_limits<int>::lowest() :
                std::numeric_limits<int>::max();
            MMReturn best = MMReturn{starting_heuristic, empty_action, 0};
            for (auto& action : moves)
            {
                LOG("minimax: action: " << action);
                // Apply, recurse, and unapply the action
                auto back_action = state.apply_action(action);
                auto ret = minimax(state, me, depth_remaining - 1, lower, upper);
                state.apply_back_action(back_action);

                best.states_evaluated += ret.states_evaluated;
                if (maximizing)
                {
                    // Prune
                    if (ret.heuristic > upper)
                    {
                        LOG("minimax: short circuit");
                        best.heuristic = ret.heuristic;
                        best.action = action;
                        break;
                    }
                    // Set new best
                    if (ret.heuristic > best.heuristic)
                    {
                        LOG("minimax: new best");
                        best.heuristic = ret.heuristic;
                        best.action = action;
                        if (ret.heuristic > lower)
                        {
                            lower = ret.heuristic;
                        }
                    }
                }
                else
                {
                    // Prune
                    if (ret.heuristic < lower)
                    {
                        LOG("minimax: short circuit");
                        best.heuristic = ret.heuristic;
                        best.action = action;
                        break;
                    }
                    // Set new best
                    if (ret.heuristic < best.heuristic)
                    {
                        LOG("minimax: new best");
                        best.heuristic = ret.heuristic;
                        best.action = action;
                        if (ret.heuristic < upper)
                        {
                            upper = ret.heuristic;
                        }
                    }
                }
            }
            return best;
        }
    }

    MMReturn interruptable_minimax(const State& cstate, Color me, int depth_remaining,
            int lower, int upper, std::atomic<bool> &stop)
    {
        LOG("interruptable_minimax");
        if (depth_remaining < 3)
        {
            // Revert back to uninterruptable if depth is small enough
            return minimax(cstate, me, depth_remaining, lower, upper);
        }
        // Cast away const-ness (it's ok, back_actions SHOULD return it to the original state)
        State& state = const_cast<State&>(cstate);
        
        static const Action empty_action(Position(-1, -1), Position(-1, -1), Empty);

        auto moves = state.generate_actions();
        bool stalemate = moves.empty();
        bool draw = state.draw();
        // Base case, terminal node
        if (stalemate || draw || depth_remaining == 0)
        {
            return MMReturn{heuristic(state, me, stalemate, draw), empty_action, 1};
        }
        else
        {
            bool maximizing = (state.turn % 2 ? Black : White) == me;
            // Initialize our "best" action with the worst possible action
            auto starting_heuristic = maximizing ? std::numeric_limits<int>::lowest() :
                std::numeric_limits<int>::max();
            MMReturn best = MMReturn{starting_heuristic, empty_action, 0};
            for (auto& action : moves)
            {
                // Apply, recurse, and unapply the action
                auto back_action = state.apply_action(action);
                auto ret = interruptable_minimax(state, me, depth_remaining - 1, lower, upper, stop);
                state.apply_back_action(back_action);

                best.states_evaluated += ret.states_evaluated;
                if (maximizing)
                {
                    // Prune
                    if (ret.heuristic > upper)
                    {
                        LOG("minimax: short circuit");
                        best.heuristic = ret.heuristic;
                        best.action = action;
                        break;
                    }
                    // Set new best
                    if (ret.heuristic > best.heuristic)
                    {
                        LOG("minimax: new best");
                        best.heuristic = ret.heuristic;
                        best.action = action;
                        if (ret.heuristic > lower)
                        {
                            lower = ret.heuristic;
                        }
                    }
                }
                else
                {
                    // Prune
                    if (ret.heuristic < lower)
                    {
                        LOG("minimax: short circuit");
                        best.heuristic = ret.heuristic;
                        best.action = action;
                        break;
                    }
                    // Set new best
                    if (ret.heuristic < best.heuristic)
                    {
                        LOG("minimax: new best");
                        best.heuristic = ret.heuristic;
                        best.action = action;
                        if (ret.heuristic < upper)
                        {
                            upper = ret.heuristic;
                        }
                    }
                }
                // Check if another thread wants us to stop
                //  We do this AFTER we set a new best so that we should always
                //  have a valid action ready to return.
                if (stop)
                {
                    LOG("interruptable_minimax: stop");
                    break;
                }
            }
            return best;
        }
    }

    std::vector<std::pair<Action, MMReturn>> pondering_minimax(const State& cstate,
            Color me, int depth_remaining, int lower, int upper, std::atomic<bool> &stop)
    {
        LOG("pondering_minimax");
        // Cast away const-ness (it's ok, back_actions SHOULD return it to the original state)
        State& state = const_cast<State&>(cstate);
        
        static const Action empty_action(Position(-1, -1), Position(-1, -1), Empty);

        auto moves = state.generate_actions();

        std::vector<std::pair<Action, MMReturn>> bests;
        for (auto& action : moves)
        {
            LOG("pondering_minimax: " << action);
            auto back_action = state.apply_action(action);
            bests.emplace_back(action, interruptable_minimax(state, me, depth_remaining - 1,
                        std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max(), stop));
            std::cout << bests.back().second.heuristic << std::endl; // TODO: Remove
            state.apply_back_action(back_action);
            if (stop) break;
        }
        return bests;
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
        // Endgame
        if (their_material <= 3)
        {
            // Promote the pawns!
            h += state.count_pawn_advancement(me) * 20;
            // Force moves
            h += 8;
            h -= state.pieces_by_color_and_type[!me][King][0]->moves.size();
            // Put the king in check
            h += state.is_in_check(!me) ? 5 : 0;
        }
        else
        {
            // Net checks 
            h += state.count_net_check_values(me) - state.count_net_check_values(!me);

            // Mobility
            h += (state.count_piece_moves(me) - state.count_piece_moves(!me)) * 3;
        }

        // Add dominating bonus for checkmate
        if (stalemate)
        {
            if (state.is_in_check(current)) // Checkmate
            {
                h = (current == me) ? -100000 : 100000;
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
