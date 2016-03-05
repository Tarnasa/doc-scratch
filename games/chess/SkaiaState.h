#pragma once

// A memory-efficient class for storing state information

#include <bitset>
#include <array>
#include <iostream> // TODO: Remove this

#include "SkaiaAction.h"
#include "Skaia.h"

namespace Skaia
{
    /*
    class NeoSkaia
    {
        public:
            struct Square
            {
                Piece* piece;
                std::bitset<32> checks;

                Square() {}
                Square(Piece* piece, const std::bitset<32>& checks) : piece(piece), checks(checks) {}
            };

            size_t turn;
            std::array<Piece, 32> pieces;
            std::array<Square, 8 * 8> squares;
            std::array<std::array<std::vector<Piece*>, NumberOfTypes>, 2> piecesByColorAndType;
            Piece* doubleMovedPawn; // Points to a pawn that is capturable by en-passant, or nullptr

            State() {}
            void initialize();
            */




    class State
    {
        public:
            struct Square
            {
                Type type;
                Color color;
                std::array<size_t, 2> checked;
                size_t special;

                Square() {}
                Square(Type type, Color color, size_t w_checked, size_t b_checked, size_t special) :
                    type(type), color(color), special(special) { checked[0] = w_checked; checked[1] = b_checked; }
            };

            size_t turn;
            std::array<Square, 8 * 8> squares;

            State() {}
            void initialize();

            bool inside(int rank, int file) const { return 0 <= rank && rank < 8 && 0 <= file && file < 8; }
            bool inside(const Position& pos) const { return inside(pos.rank, pos.file); }
            const Square& at(int rank, int file) const
            {
                std::cout << "at: " << rank << ", " << file << std::endl;
                return squares[rank * 8 + file];
            }
            const Square& at(const Position& pos) const
            {
                std::cout << "at: " << pos.rank << ", " << pos.file << std::endl;
                return squares[pos.rank * 8 + pos.file];
            }
            Square& at(int rank, int file)
            {
                std::cout << "at: " << rank << ", " << file << std::endl;
                return squares[rank * 8 + file];
            }
            Square& at(const Position& pos)
            {
                std::cout << "at: " << pos.rank << ", " << pos.file << std::endl;
                return squares[pos.rank * 8 + pos.file];
            }

            int distance_from_back_rank(bool color, int rank) const
            {
                return color ? 7 - rank : rank;
            }

            void add_piece(const Position& pos, Type type, Color color)
            {
                // TODO:
                // Check for blocking other pieces lines
                // Check for which squares the newly placed piece checks
                at(pos.rank, pos.file) = Square(type, color, 0u, 0u, 0u);
            }

            void remove_piece(const Position& pos)
            {
                // TODO:
                // Remove lines from previous piece
                // Check for new sight lines
                at(pos.rank, pos.file) = Square(Empty, White, 0u, 0u, 0u);
            }

            void move_piece(const Position& from, const Position& to)
            {
                Type type = at(from).type;
                Color color = at(from).color;
                remove_piece(from);
                add_piece(to, type, color);
            }

            void apply_action(const Action& action);

            bool valid_move(const Square& from, const Position& to) const
            {
                return inside(to) && (at(to).type == Empty || at(to).color != from.color);
            }
            bool empty(const Position& pos) const
            {
                std::cout << "empty? " << pos.rank << ", " << pos.file << std::endl;
                return inside(pos) && at(pos).type == Empty;
            }
            bool attackable(const Square& from, const Position& to) const
            {
                return inside(to) && at(to).type != Empty && at(to).color != from.color;
            }

            void pawn_move_with_promotions(const Square& square, const Position& from, const Position& to, std::vector<Action>& actions) const
            {
                std::cout << "Promotions" << std::endl;
                if (to.rank == (square.color ? 7 : 0))
                {
                    actions.emplace_back(from, to, Bishop);
                    actions.emplace_back(from, to, Knight);
                    actions.emplace_back(from, to, Rook);
                    actions.emplace_back(from, to, Queen);
                }
                else
                {
                    actions.emplace_back(from, to, Empty);
                }
            }

            void possible_pawn_moves(const Square& square, const Position& pos, std::vector<Action>& actions) const
            {
                std::cout << "Pawn" << std::endl;
                int direction = square.color ? 1 : -1;
                // Move forward
                std::cout << "Move forward" << std::endl;
                Position new_pos(pos + Position(direction, 0));
                std::cout << "new_pos" << std::endl;
                if (empty(new_pos)) pawn_move_with_promotions(square, pos, new_pos, actions);
                std::cout << "Double move check" << std::endl;
                // Move forward twice on first move
                if (pos.rank == (square.color ? 6 : 1))
                {
                    new_pos = pos + Position(direction * 2, 0);
                    if (empty(pos + Position(direction, 0)) && empty(new_pos)) actions.emplace_back(pos, new_pos, Empty);
                }
                std::cout << "Attack" << std::endl;
                // Attack + En Passant
                new_pos = pos + Position(direction, -1);
                if (attackable(square, new_pos)) pawn_move_with_promotions(square, pos, new_pos, actions);
                if (attackable(square, Position(pos.rank, new_pos.file)) && at(pos.rank, new_pos.file).special == (turn - 1) && empty(new_pos)) actions.emplace_back(pos, new_pos, King);
                new_pos = pos + Position(direction, +1);
                if (attackable(square, new_pos)) pawn_move_with_promotions(square, pos, new_pos, actions);
                if (attackable(square, Position(pos.rank, new_pos.file)) && at(pos.rank, new_pos.file).special == (turn - 1) && empty(new_pos)) actions.emplace_back(pos, new_pos, King);
            }

            void line_moves(const Square& square, const Position& from, const Position& delta, std::vector<Action>& actions) const
            {
                for (auto d = 1u; d < 8; ++d)
                {
                    Position new_pos = from + delta * d;
                    if (inside(new_pos))
                    {
                        if (empty(new_pos))
                        {
                            actions.emplace_back(from, new_pos, Empty);
                        }
                        else if (attackable(square, new_pos))
                        {
                            actions.emplace_back(from, new_pos, Empty);
                            break;
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }

            void possible_bishop_moves(const Square& square, const Position& from, std::vector<Action>& actions) const
            {
                line_moves(square, from, Position(1, 1), actions);
                line_moves(square, from, Position(-1, 1), actions);
                line_moves(square, from, Position(1, -1), actions);
                line_moves(square, from, Position(-1, -1), actions);
            }

            void possible_rook_moves(const Square& square, const Position& from, std::vector<Action>& actions) const
            {
                line_moves(square, from, Position(1, 0), actions);
                line_moves(square, from, Position(0, 1), actions);
                line_moves(square, from, Position(-1, 0), actions);
                line_moves(square, from, Position(0, -1), actions);
            }

            void possible_knight_moves(const Square& square, const Position& from, std::vector<Action>& actions) const
            {
                Position new_pos = from + Position(-2, +1);
                if (valid_move(square, new_pos)) actions.emplace_back(from, new_pos, Empty);
                new_pos = from + Position(-1, +2);
                if (valid_move(square, new_pos)) actions.emplace_back(from, new_pos, Empty);
                new_pos = from + Position(+1, +2);
                if (valid_move(square, new_pos)) actions.emplace_back(from, new_pos, Empty);
                new_pos = from + Position(+2, +1);
                if (valid_move(square, new_pos)) actions.emplace_back(from, new_pos, Empty);
                new_pos = from + Position(+2, -1);
                if (valid_move(square, new_pos)) actions.emplace_back(from, new_pos, Empty);
                new_pos = from + Position(+1, -2);
                if (valid_move(square, new_pos)) actions.emplace_back(from, new_pos, Empty);
                new_pos = from + Position(-1, -2);
                if (valid_move(square, new_pos)) actions.emplace_back(from, new_pos, Empty);
                new_pos = from + Position(-2, -1);
                if (valid_move(square, new_pos)) actions.emplace_back(from, new_pos, Empty);
            }

            std::vector<Action> generate_actions() const
            {
                std::vector<Action> actions;
                // TODO: Optimize
                // TODO: Consider Check
                for (auto rank = 0u; rank < 8; ++rank)
                {
                    for (auto file = 0u; file < 8; ++file)
                    {
                        Position pos(rank, file);
                        const Square& square = at(pos);
                        if (square.type != Empty && square.color == turn % 2)
                        {
                            std::cout << "Type: " << square.type << std::endl;
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

            /*
            Position is_in_check_from_direction_rook(Color color, const Position& pos, const Position& delta)
            {
                for (int i = 0; i < 8; ++i)
                {
                    Position new_pos = pos + delta * i;
                    if (!inside(pos)) break;
                    if (at(pos).type == Rook || at(pos).type == Queen && at(pos).type != Empty && at(pos).color != color)
                    {
                        return true;
                    }
                }
            }

            bool is_in_check(Color color) const
            {
                // Find King
                Square king;
                Position king_pos;
                for (auto i = 0u; i < 8 * 8; ++i)
                {
                    if (square[i].type == King && square[i].color == color)
                    {
                        king = squares[i];
                        king_pos = Position(i / 8, i % 8);
                    }
                }
                // Search outward in each direction to find attackers
                for (int i = 0; i < 8; ++i)
                {


            }
            */
    };

    /*
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
    */
}

