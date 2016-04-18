#include "SkaiaState.h"

#include <algorithm>
#include <iterator>
#include <set>

// This file has the implementations for all the generally
//  uninteresting functions of the State class.

namespace Skaia
{
    const State::Square& State::at(int rank, int file) const
    {
        return squares[rank * 8 + file];
    }
    const State::Square& State::at(const Position& pos) const
    {
        return squares[pos.rank * 8 + pos.file];
    }
    State::Square& State::at(int rank, int file)
    {
        return squares[rank * 8 + file];
    }
    State::Square& State::at(const Position& pos)
    {
        return squares[pos.rank * 8 + pos.file];
    }

    bool State::empty(const Position& pos) const
    {
        return inside(pos) && at(pos).piece == nullptr;
    }
    bool State::canTake(const Piece* piece, const Position& pos) const
    {
        return inside(pos) && (at(pos).piece == nullptr || at(pos).piece->color != piece->color);
    }
    bool State::canKill(const Piece* piece, const Position& pos) const
    {
        return inside(pos) && at(pos).piece != nullptr && at(pos).piece->color != piece->color;
    }

    void State::check_piece(const Piece* piece, bool check)
    {
        switch (piece->type)
        {
            case Pawn: check_pawn(piece, check); break;
            case Bishop: check_bishop(piece, check); break;
            case Knight: check_knight(piece, check); break;
            case Rook: check_rook(piece, check); break;
            case Queen: check_bishop(piece, check); check_rook(piece, check); break;
            case King: check_king(piece, check); break;
        }
    }

    void State::check_ray(const Piece* piece, const Position& delta, bool check)
    {
        LOG("check_ray");
        ray_action(piece->pos, delta, [&, this](const Position& pos) {
            this->at(pos).checks[piece->id] = check;
        });
    }

    void State::check_pos(const Piece* piece, const Position& pos, bool check)
    {
        if (inside(pos)) at(pos).checks[piece->id] = check;
    }

    void State::check_pawn(const Piece* piece, bool check)
    {
        LOG("check_pawn");
        int direction = piece->color ? 1 : -1;
        check_pos(piece, piece->pos + Position(direction, 1), check);
        check_pos(piece, piece->pos + Position(direction, -1), check);
    }

    void State::check_bishop(const Piece* piece, bool check)
    {
        LOG("check_bishop");
        check_ray(piece, Position(1, 1), check);
        check_ray(piece, Position(-1, 1), check);
        check_ray(piece, Position(1, -1), check);
        check_ray(piece, Position(-1, -1), check);
    }

    void State::check_rook(const Piece* piece, bool check)
    {
        LOG("check_rook");
        check_ray(piece, Position(1, 0), check);
        check_ray(piece, Position(0, 1), check);
        check_ray(piece, Position(-1, 0), check);
        check_ray(piece, Position(0, -1), check);
    }

    void State::check_knight(const Piece* piece, bool check)
    {
        LOG("check_knight");
        check_pos(piece, piece->pos + Position(2, 1), check);
        check_pos(piece, piece->pos + Position(1, 2), check);
        check_pos(piece, piece->pos + Position(-1, 2), check);
        check_pos(piece, piece->pos + Position(-2, 1), check);
        check_pos(piece, piece->pos + Position(-2, -1), check);
        check_pos(piece, piece->pos + Position(-1, -2), check);
        check_pos(piece, piece->pos + Position(1, -2), check);
        check_pos(piece, piece->pos + Position(2, -1), check);
    }

    void State::check_king(const Piece* piece, bool check)
    {
        LOG("check_king");
        check_pos(piece, piece->pos + Position(0, 1), check);
        check_pos(piece, piece->pos + Position(1, 1), check);
        check_pos(piece, piece->pos + Position(1, 0), check);
        check_pos(piece, piece->pos + Position(1, -1), check);
        check_pos(piece, piece->pos + Position(0, -1), check);
        check_pos(piece, piece->pos + Position(-1, -1), check);
        check_pos(piece, piece->pos + Position(-1, 0), check);
        check_pos(piece, piece->pos + Position(-1, 1), check);
    }

    void State::try_take(const Piece* piece, const Position& to, std::vector<Action>& actions) const
    {
        if (canTake(piece, to)) actions.emplace_back(piece->pos, to, Empty);
    }

    void State::possible_piece_moves(const Piece* piece, std::vector<Action>& actions) const
    {
        switch (piece->type)
        {
            case Pawn: possible_pawn_moves(piece, actions); break;
            case Bishop: possible_bishop_moves(piece, actions); break;
            case Knight: possible_knight_moves(piece, actions); break;
            case Rook: possible_rook_moves(piece, actions); break;
            case Queen: possible_bishop_moves(piece, actions); possible_rook_moves(piece, actions); break;
            case King: possible_king_moves(piece, actions); break;
        }
    }

    void State::pawn_move_with_promotions(const Piece* piece, const Position& to, std::vector<Action>& actions) const
    {
        if (to.rank == (piece->color ? 7 : 0))
        {
            actions.emplace_back(piece->pos, to, Queen);
            // We aint no foo'
            actions.emplace_back(piece->pos, to, Bishop);
            actions.emplace_back(piece->pos, to, Knight);
            actions.emplace_back(piece->pos, to, Rook);
        }
        else
        {
            actions.emplace_back(piece->pos, to, Empty);
        }
    }

    void State::possible_pawn_moves(const Piece* piece, std::vector<Action>& actions) const
    {
        int direction = piece->color ? 1 : -1;
        // Move forward
        Position new_pos(piece->pos + Position(direction, 0));
        if (empty(new_pos)) pawn_move_with_promotions(piece, new_pos, actions);
        // Move forward twice on first move
        if (piece->special) // special==true when piece has never moved
        {
            new_pos = piece->pos + Position(direction * 2, 0);
            if (empty(piece->pos + Position(direction, 0)) && empty(new_pos)) actions.emplace_back(piece->pos, new_pos, Pawn);
        }
        // Attack + En Passant
        new_pos = piece->pos + Position(direction, -1);
        if (canKill(piece, new_pos)) pawn_move_with_promotions(piece, new_pos, actions);
        if (canKill(piece, Position(piece->pos.rank, new_pos.file)) && at(piece->pos.rank, new_pos.file).piece == double_moved_pawn && empty(new_pos)) actions.emplace_back(piece->pos, new_pos, King);
        new_pos = piece->pos + Position(direction, +1);
        if (canKill(piece, new_pos)) pawn_move_with_promotions(piece, new_pos, actions);
        if (canKill(piece, Position(piece->pos.rank, new_pos.file)) && at(piece->pos.rank, new_pos.file).piece == double_moved_pawn && empty(new_pos)) actions.emplace_back(piece->pos, new_pos, King);
    }

    void State::line_moves(const Piece* piece, const Position& delta, std::vector<Action>& actions) const
    {
        ray_action_const(piece, delta, [&](const Position& new_pos) {
            actions.emplace_back(piece->pos, new_pos, Empty);
        });
    }

    void State::possible_bishop_moves(const Piece* piece, std::vector<Action>& actions) const
    {
        line_moves(piece, Position(1, 1), actions);
        line_moves(piece, Position(-1, 1), actions);
        line_moves(piece, Position(1, -1), actions);
        line_moves(piece, Position(-1, -1), actions);
    }

    void State::possible_rook_moves(const Piece* piece, std::vector<Action>& actions) const
    {
        line_moves(piece, Position(1, 0), actions);
        line_moves(piece, Position(0, 1), actions);
        line_moves(piece, Position(-1, 0), actions);
        line_moves(piece, Position(0, -1), actions);
    }

    void State::possible_knight_moves(const Piece* piece, std::vector<Action>& actions) const
    {
        try_take(piece, piece->pos + Position(-2, +1), actions);
        try_take(piece, piece->pos + Position(-1, +2), actions);
        try_take(piece, piece->pos + Position(+1, +2), actions);
        try_take(piece, piece->pos + Position(+2, +1), actions);
        try_take(piece, piece->pos + Position(+2, -1), actions);
        try_take(piece, piece->pos + Position(+1, -2), actions);
        try_take(piece, piece->pos + Position(-1, -2), actions);
        try_take(piece, piece->pos + Position(-2, -1), actions);
    }

    void State::possible_king_moves(const Piece* piece, std::vector<Action>& actions) const
    {
        try_take(piece, piece->pos + Position(0, 1), actions);
        try_take(piece, piece->pos + Position(1, 1), actions);
        try_take(piece, piece->pos + Position(1, 0), actions);
        try_take(piece, piece->pos + Position(1, -1), actions);
        try_take(piece, piece->pos + Position(0, -1), actions);
        try_take(piece, piece->pos + Position(-1, -1), actions);
        try_take(piece, piece->pos + Position(-1, 0), actions);
        try_take(piece, piece->pos + Position(-1, 1), actions);
        // Castle
        if (piece->special && at(piece->pos).checked_by_color(!piece->color ? Black : White))
        {
            auto empty_and_unchecked = [this, &piece, &actions](const Position& pos) {
                return empty(pos) && !at(pos).checked_by_color(!piece->color ? Black : White);
            };
            // Queen-side castle
            if (empty_and_unchecked(piece->pos + Position(0, -1)) &&
                    empty_and_unchecked(piece->pos + Position(0, -2)) &&
                    empty_and_unchecked(piece->pos + Position(0, -3)) &&
                    at(piece->pos + Position(0, -4)).piece->type == Rook &&
                    at(piece->pos + Position(0, -4)).piece->special &&
                    at(piece->pos + Position(0, -4)).checked_by_color(!piece->color ? Black : White))
            {
                actions.emplace_back(piece->pos, piece->pos + Position(0, -2), King);
            }
            // King-side castle
            else if (empty_and_unchecked(piece->pos + Position(0, 1)) &&
                    empty_and_unchecked(piece->pos + Position(0, 2)) &&
                    at(piece->pos + Position(0, 3)).piece->type == Rook &&
                    at(piece->pos + Position(0, 3)).piece->special &&
                    at(piece->pos + Position(0, 3)).checked_by_color(!piece->color ? Black : White))
            {
                actions.emplace_back(piece->pos, piece->pos + Position(0, 2), King);
            }
        }
    }
}

