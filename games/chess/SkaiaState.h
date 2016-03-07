#pragma once

// A memory-efficient class for storing state information

#include <bitset>
#include <array>
#include <iostream> // TODO: Remove this

#include "SkaiaAction.h"
#include "SkaiaPiece.h"
#include "Skaia.h"

namespace Skaia
{
    class State
    {
        public:
            struct Square
            {
                Piece* piece;
                std::bitset<32> checks;

                Square() : piece(nullptr), checks() {}
                Square(Piece* piece) : piece(piece) {}
                Square(Piece* piece, const std::bitset<32>& checks) : piece(piece), checks(checks) {}

                bool checked_by_color(Color color) const
                {
                    return checks.to_ulong() & (color ? 0x0000ffff : 0xffff0000);
                }
            };

            size_t turn;
            std::array<Piece, 32> pieces;
            std::array<Square, 8 * 8> squares;
            std::array<std::array<std::vector<Piece*>, NumberOfTypes>, 2> pieces_by_color_and_type;
            Piece* double_moved_pawn; // Points to a pawn that is capturable by en-passant, or nullptr

            State();
            State(const State& source);

            // Check if a position is on the board
            static bool inside(int rank, int file) { return 0 <= rank && rank < 8 && 0 <= file && file < 8; }
            static bool inside(const Position& pos) { return inside(pos.rank, pos.file); }

            // Access a square
            const Square& at(int rank, int file) const;
            const Square& at(const Position& pos) const;
            Square& at(int rank, int file);
            Square& at(const Position& pos);

            // Check if a position is good for a piece to move to
            bool empty(const Position& pos) const;
            bool canTake(const Piece* piece, const Position& pos) const;
            bool canKill(const Piece* piece, const Position& pos) const;

            // Ray-casting functions
            template<typename F> void ray_action(const Piece* piece, const Position& delta, F func)
            {
                if (delta.rank == 0 && delta.file == 0)
                {
                    std::cout << "No delta!" << std::endl;
                    return;
                }
                Position new_pos = piece->pos;
                while (true)
                {
                    new_pos += delta;
                    if (!canTake(piece, new_pos)) break;
                    func(new_pos);
                    if (!empty(new_pos)) break;
                }
            }
            template<typename F> void ray_action(const Position& from, const Position& delta, F func)
            {
                if (delta.rank == 0 && delta.file == 0)
                {
                    std::cout << "No delta!" << std::endl;
                    return;
                }
                Position new_pos = from;
                while (true)
                {
                    new_pos += delta;
                    if (!inside(new_pos)) break;
                    func(new_pos);
                    if (!empty(new_pos)) break;
                }
            }
            template<typename F> void ray_action_const(const Piece* piece, const Position& delta, F func) const
            {
                if (delta.rank == 0 && delta.file == 0) return;
                Position new_pos = piece->pos;
                while (true)
                {
                    new_pos += delta;
                    if (!canTake(piece, new_pos)) break;
                    func(new_pos);
                    if (!empty(new_pos)) break;
                }
            }
            Position ray(const Piece* piece, const Position& delta) const;

            // Functions for setting check
            void check_ray(const Piece* piece, const Position& delta, bool check);
            void check_pos(const Piece* piece, const Position& pos, bool check);
            void check_pawn(const Piece* piece, bool check);
            void check_bishop(const Piece* piece, bool check);
            void check_rook(const Piece* piece, bool check);
            void check_knight(const Piece* piece, bool check);
            void check_king(const Piece* piece, bool check);

            // Functions for moving pieces around the board
            void place_piece(Piece* piece, const Position& pos);
            void remove_piece(const Piece* piece);
            void move_piece(const Position& from, const Position& to);

            int distance_from_back_rank(Color color, int rank) const
            {
                return color ? 7 - rank : rank;
            }

            void apply_action(const Action& action);

            // Functions for generating moves
            bool is_in_check(Color color) const;
            std::vector<Action> generate_actions() const;

            void try_take(const Piece* piece, const Position& to, std::vector<Action>& actions) const;
            void pawn_move_with_promotions(const Piece* piece, const Position& to, std::vector<Action>& actions) const;
            void possible_pawn_moves(const Piece* piece, std::vector<Action>& actions) const;
            void line_moves(const Piece* piece, const Position& delta, std::vector<Action>& actions) const;
            void possible_bishop_moves(const Piece* piece, std::vector<Action>& actions) const;
            void possible_rook_moves(const Piece* piece, std::vector<Action>& actions) const;
            void possible_knight_moves(const Piece* piece, std::vector<Action>& actions) const;
            void possible_king_moves(const Piece* piece, std::vector<Action>& actions) const;
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

std::ostream& operator<<(std::ostream& out, const Skaia::State& state);


