#pragma once

#include "Skaia.h"
#include "SkaiaPiece.h"
#include "SkaiaSimpleSmallState.h"

// This class should be used to revert a Skaia::State to the previous state,
//  so that so many copies of Skaia::State don't have to be made.

namespace Skaia
{
    class BackAction
    {
        public:
            Type type; // To signify special moves like promotion
            Piece actor_piece;
            Piece taken_piece;
            SimpleSmallState old_state; // The eight-oldest state to be re-added to history
            Piece* previous_double_moved_pawn;
            int since_pawn_or_capture;
    };
}

