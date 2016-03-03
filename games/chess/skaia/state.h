#pragma once

// A memory-efficient class for storing state information

#include <bitset>
#include <array>

#include "action.h"
#include "common.h"

namespace Skaia
{
    class State
    {
        public:
            struct Square
            {
                Type type;
                Color color;
                std::array<size_t, 2> checked;
                size_t special;

                Square(Type type, bool color, size_t w_checked, size_t b_checked, size_t special);
            };

            size_t turn;
            std::array<Square, 8 * 8> squares;

            State() {}

            bool inside(size_t rank, size_t file) const { return 0 <= rank && rank < 8 && 0 <= file && file < 8; }
            bool inside(const Position& pos) const { return inside(pos.rank, pos.file); }
            Square& at(size_t rank, size_t file) const
            {
                return squares[rank * 8 + file];
            }
            Square& at(const Position& pos) const
            {
                return square[pos.rank * 8 + pos.file];
            }

            size_t distance_from_back_rank(bool color, size_t rank) const
            {
                return color ? 7 - rank : rank;
            }

            void add_piece(const Position& pos, Type type, bool color)
            {
                // TODO:
                // Check for blocking other pieces lines
                // Check for which squares the newly placed piece checks
                at(pos.rank, pos.file) = Square(type, color, 0, 0, 0);
            }

            void remove_piece(const Position& pos)
            {
                // TODO:
                // Remove lines from previous piece
                // Check for new sight lines
                at(pos.rank, pos.file) = Square(Empty, false, 0, 0, 0);
            }

            void move_piece(const Position& from, const Position& to)
            {
                Type type = at(from).type;
                bool color = at(from).color;
                remove_piece(from);
                add_piece(to, type, color);
            }

            void apply_action(const Action& action)
            {
                // TODO: Make the common case fast
                Square& from = at(action.from);
                Square& to = at(action.to);
                // Special cases
                if (action.promotion == King)
                {
                    // Promotion
                    if (from.type == Pawn && distance_from_back_rank(from.color, to.rank) == 7)
                    {
                        Color color = from.color;
                        move_piece(action.from, action.to);
                        remove_piece(action.to);
                        add_piece(action.to, action.promotion, color);
                    }
                    // Castling
                    else if (from.type == King && std::abs(from.file - to.file) == 2)
                    {
                        move_piece(action.from, action.to);
                        Position rook_from(from.rank, to.file == 2 ? 0 : 7);
                        Position rook_to(from.rank, to.file == 2 ? 3 : 5);
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

            bool valid_move(const Square& from, const Position& to)
            {
                return inside(to) && (at(to).type == Empty || at(to).color != from.color);
            }
            bool empty(const Position& pos)
            {
                return inside(pos) && at(pos).type == Empty;
            }
            bool attackable(const Square& from, const Position& to)
            {
                return inside(to) && at(to).type != Empty && at(to).color != from.color;
            }

            void pawn_move_with_promotions(const Square& square, const Position& from, const Position& to, std::vector<Action>& actions) const
            {
                if (to.rank == square.color ? 7 : 0)
                {
                    acitons.emplace_back(from, to, Bishop);
                    acitons.emplace_back(from, to, Knight);
                    acitons.emplace_back(from, to, Rook);
                    acitons.emplace_back(from, to, Queen);
                }
                else
                {
                    actions.emplace_back(from, to, Empty);
                }
            }

            void possible_pawn_moves(const Square& square, const Position& pos, std::vector<Action>& actions) const
            {
                int direction = square.color ? 1 : -1;
                // Move forward
                Position new_pos = pos + Position(direction, 0);
                if (empty(square, new_pos)) pawn_move_with_promotions(pos, new_pos, actions);
                // Move forward twice on first move
                if (rank == square.color ? 6 : 1)
                {
                    new_pos = pos + Position(direction * 2, 0);
                    if (empty(new_pos)) actions.emplace_back(pos, new_pos, Empty);
                }
                // Attack + En Passant
                new_pos = pos + Position(direction, -1);
                if (attackable(square, new_pos)) pawn_move_with_promotions(square, pos, new_pos, actions);
                if (attackable(square, Position(pos.rank, new_pos.file)) && empty(new_pos)) actions.emplace_back(pos, new_pos, King);
                new_pos = pos + Position(direction, +1);
                if (attackable(square, new_pos)) pawn_move_with_promotions(square, pos, new_pos, actions);
                if (attackable(square, Position(pos.rank, new_pos.file)) && empty(new_pos)) actions.emplace_back(pos, new_pos, King);
            }

            void possible_bishop_moves(const Square& square, const Position& pos, std::vector<Action>& actions) const
            {
                for (auto d = 1u; d < 8; ++d)
                {
                    Position new_pos = from + Position(d, d);
                    if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                    new_pos = from + Position(-d, d);
                    if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                    new_pos = from + Position(d, -d);
                    if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                    new_pos = from + Position(-d, -d);
                    if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                }
            }

            void possible_rook_moves(const Square& square, const Position& pos, std::vector<Action>& actions) const
            {
                // TODO: Optimize into 4 for loops
                for (auto d = 1u; d < 8; ++d)
                {
                    Position new_pos = from + Position(d, 0);
                    if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                    new_pos = from + Position(0, d);
                    if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                    new_pos = from + Position(-d, 0);
                    if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                    new_pos = from + Position(0, -d);
                    if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                }
            }

            void possible_knight_moves(const Square& square, const Position& pos, std::vector<Action>& actions) const
            {
                Position new_pos = from + Position(-2, +1);
                if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                new_pos = from + Position(-1, +2);
                if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                new_pos = from + Position(+1, +2);
                if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                new_pos = from + Position(+2, +1);
                if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                new_pos = from + Position(+2, -1);
                if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                new_pos = from + Position(+1, -2);
                if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                new_pos = from + Position(-1, -2);
                if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
                new_pos = from + Position(-2, -1);
                if (valid_move(from, new_pos)) actions.emplace_back(pos, new_pos, Empty);
            }

            std::vector<Action> possible_moves() const
            {
                std::vector<Action> actions;
                // TODO: Optimize
                for (auto rank = 0u; rank < 8; ++rank)
                {
                    for (auto file = 0u; file < 8 ++file)
                    {
                        Position pos(rank, file);
                        Square& square = at(pos);
                        if (square.type != Empty && square.color == turn % 2)
                        {
                            switch (square.type)
                            {
                                case Pawn: possible_pawn_moves(square, pos, actions); break;
                                case Bishop: possible_bishop_moves(square, pos, actions); break;
                                case Knight: possible_knight_moves(square, pos, actions); break;
                                case Rook: possible_rook_moves(square, pos, actions); break;
                                case Queen:
                                    possible_bishop_moves(square, pos, actions);
                                    possible_rook_moves(square, pos, actions);
                                    break;
                            }
                        }
                    }
                }
                return actions;
            }

            struct IterableActions
            {
                State state;
                PossibleActionsIterator begin();
                PossibleActionsIterator end();
            }

            struct PossibleActionsIterator
            {
                Action action;
                
                PossibleActionsIterator() : action(Position(0, 0), Position(0, 0), King) {}
                PossibleActionsIterator(const Action& action) : action(action);
                bool operator!=(const PossibleActionsIterator& other) const
                {
                    return action.promotion != action.promotion;
                }
            }
    }

    class SmallState
    {
        public:
            constexpr size_t type_bitsize = 3; // Empty,Pawn,Rook,Knight,Bishop,Queen,King
            constexpr size_t color_bitsize = 1; // 0=White,1=Black
            constexpr size_t w_checked_bitsize = 2; // Number of white pieces checking this square (max 4)
            constexpr size_t b_checked_bitsize = 2; // Number of black pieces checking this square (max 4)
            constexpr size_t special_bitsize = 1; // For pawns, whether the piece has just moved.  For kings and rooks Whether the piece has ever moved.
            constexpr size_t square_bitsize = type_bitsize + color_bitsize + w_checked_bitsize + b_checked_bitsize + special_bitsize;
            constexpr size_t data_bitsize = square_bitsize * 8 * 8;
            std::bitset<data_bitsize> data;
    };
}

