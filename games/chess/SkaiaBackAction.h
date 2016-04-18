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
            Piece actor; // The previous state of the acting piece
            Piece taken; // The previous state of the taken piece (if one was taken), otherwise it is the default-constructed Piece
            uint64_t old_action; // The eighth-oldest action to be re-added to history, or 0 if there isn't enough history
            int double_moved_pawn_id; // -1 if there was no previously double moved pawn
            int since_pawn_or_capture;
            bool captured;
    };
}

