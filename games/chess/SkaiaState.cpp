#include "SkaiaState.h"

namespace Skaia
{
    void State::initialize()
    {
        turn = 0;
        // Empty squares
        for (auto rank = 2u; rank <= 5; ++rank)
        {
            for (auto file = 0u; file < 8; ++file)
            {
                at(rank, file) = Square(Empty, White, 0, 0, 0);
            }
        }
        // Pieces
        static const std::vector<Type> order = {Rook, Knight, Bishop, Queen, King, Bishop, Knight, Rook};
        for (auto file = 0u; file < 8; ++file)
        {
            at(1, file) = Square(Pawn, Black, 0, 0, 0);
            at(6, file) = Square(Pawn, White, 0, 0, 0);
            at(0, file) = Square(order[file], Black, 0, 0, 0);
            at(7, file) = Square(order[file], White, 0, 0, 0);
        }
    }

    void State::apply_action(const Action& action)
    {
        // TODO: Make the common case fast
        Square& from = at(action.from);
        Square& to = at(action.to);
        // Special cases
        if (action.promotion == King)
        {
            // Promotion
            if (from.type == Pawn && distance_from_back_rank(from.color, action.to.rank) == 7)
            {
                Color color = from.color;
                move_piece(action.from, action.to);
                remove_piece(action.to);
                add_piece(action.to, action.promotion, color);
            }
            // Castling
            else if (from.type == King && std::abs(static_cast<int>(action.from.file - action.to.file)) == 2)
            {
                move_piece(action.from, action.to);
                Position rook_from(action.from.rank, action.to.file == 2 ? 0 : 7);
                Position rook_to(action.from.rank, action.to.file == 2 ? 3 : 5);
                move_piece(rook_from, rook_to);
            }
            // En passant
            else if (from.type == Pawn)
            {
                move_piece(action.from, action.to);
                remove_piece(Position(action.from.rank, action.to.file));
            }
        }
        // Normal move
        else
        {
            if (to.type != Empty)
                remove_piece(action.to);
            move_piece(action.from, action.to);
            if (to.type == Pawn || to.type == King || to.type == Rook)
                to.special = turn;
        }
    }
}
