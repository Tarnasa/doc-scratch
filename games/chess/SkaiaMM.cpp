#include "SkaiaMM.h"

namespace Skaia
{
    MMReturn Minimax(const State& state, int depth_remaining)
    {
        if (depth_remaining == 0)
        {
            return MMReturn{material(state), Action(Position(-1, -1), Position(-1, -1), Empty), 
    }

    int material(const State& state, Color me)
    {
        Color current = state.turn % 2 ? Black : White;
        int h = 0;
        // Account for material
        h += state.material(me) - state.material(!me ? Black : White);
        // Add bonusses for check
        h -= state.is_in_check(me) ? 20 : 0;
        h += state.is_in_check(!me ? Black : White) ? 20 : 0;
        return h;
    }
}
