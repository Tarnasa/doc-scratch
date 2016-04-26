#pragma once

// A class representing a board state along with functions for interacting with that state.

#include <bitset>
#include <array>
#include <iostream>
#include <iomanip>
#include <cstdint>

#include <boost/circular_buffer.hpp>

#include "SkaiaAction.h"
#include "SkaiaBackAction.h"
#include "SkaiaSimpleSmallState.h"
#include "SkaiaPiece.h"
#include "Skaia.h"

#include "Zobrist.h"

namespace Skaia
{
    class State
    {
        public:
            // A square tile on the chess board
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

                // For testing
                bool operator==(const Square& rhs) const;
            };

            size_t turn;
            std::array<Piece, 32> pieces;
            std::array<Square, 8 * 8> squares;
            std::array<std::array<std::vector<Piece*>, NumberOfTypes>, 2> pieces_by_color_and_type;
            Piece* double_moved_pawn; // Points to the one pawn that is capturable by en-passant, or nullptr
            boost::circular_buffer<uint64_t> history; // For detecting draws by repeat
            int since_pawn_or_capture; // For detecting draws by no pawn move or piece captured
            bool captured; // Whether a piece was captured on the last move.
            //Zobrist zobrist; // Hash board state

            // Default constructor initializes state to the beginning of a normal chess game
            State();
            State(const State& source);

            // Generate a list of valid moves for the current player
            std::vector<Action> generate_actions() const;

            // Chenge the current state by applying an action
            BackAction apply_action(const Action& action);
            void apply_back_action(const BackAction& action);

            // Detect draw
            bool draw() const;

            // Detect quiescent state
            bool quiescent() const;

            // Heuristic functions
            int material(Color color) const;
            int count_net_checks(Color color) const;
            int count_net_check_values(Color color) const;
            int count_pawn_advancement(Color color) const;
            int count_piece_moves(Color color) const;
            int count_piece_moves_value(Color color) const;

            // Access a square
            const Square& at(int rank, int file) const;
            const Square& at(const Position& pos) const;
            Square& at(int rank, int file);
            Square& at(const Position& pos);

        public:
            // Check if a position is on the board
            static bool inside(int rank, int file) { return 0 <= rank && rank < 8 && 0 <= file && file < 8; }
            static bool inside(const Position& pos) { return inside(pos.rank, pos.file); }

            // Check if a position is good for a piece to move to
            bool empty(const Position& pos) const;
            bool canTake(const Piece* piece, const Position& pos) const; // Position is empty or contains an enemy piece
            bool canKill(const Piece* piece, const Position& pos) const; // Position contains an enemy piece

            // Ray-casting functions
            // These functions apply a func to all the tiles starting at position + delta and
            //  moving the direction of delta until the given piece cannot reach the
            //  position in the current turn.
            // For example, if a queen at position (0,0) is used as the starting piece,
            //  and (1,0) is given as the delta, then func() is called with all positions
            //  from (1,0) to (7,0).
            // If an enemy pawn were at position (5,0) then only positions (1,0) to (5,0)
            //  would be considered.
            // If a friendly pawn were at position (5,0) then only positions (1,0) to (4,0)
            //  would be considered.
            template<typename F> void ray_action(const Piece* piece, const Position& delta, F func)
            {
                Position new_pos = piece->pos;
                while (true)
                {
                    new_pos += delta;
                    if (!canTake(piece, new_pos)) break;
                    func(new_pos);
                    if (!empty(new_pos)) break;
                }
            }
            template<typename F> void ray_action_const(const Piece* piece, const Position& delta, F func) const
            {
                Position new_pos = piece->pos;
                while (true)
                {
                    new_pos += delta;
                    if (!canTake(piece, new_pos)) break;
                    func(new_pos);
                    if (!empty(new_pos)) break;
                }
            }
            // This version of raw_action considers all pieces to be enemy pieces, and so
            //  will apply func() to all positions including the first encountered piece,
            //  whether friend or foe.
            template<typename F> void ray_action(const Position& from, const Position& delta, F func)
            {
                if (delta.rank == 0 && delta.file == 0)
                {
                    std::cerr << "No delta!" << std::endl;
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

            // Functions for setting check
            void check_piece(const Piece* piece, bool check);
            void check_ray(const Piece* piece, const Position& delta, bool check);
            void check_pos(const Piece* piece, const Position& pos, bool check);
            void check_pawn(const Piece* piece, bool check);
            void check_bishop(const Piece* piece, bool check);
            void check_rook(const Piece* piece, bool check);
            void check_knight(const Piece* piece, bool check);
            void check_king(const Piece* piece, bool check);

            // Functions for moving pieces around the board
            void place_piece(Piece* piece, const Position& pos);
            void remove_piece(Piece* piece); // Temporarily take the piece off the board
            void kill_piece(Piece* piece); // Make it dead
            void move_piece(const Position& from, const Position& to);

            // Functions for generating moves
            bool is_in_check(Color color) const;
            // These functions all take a vector<Action> by reference and add moves this vector
            // TODO: Make them take an output iterator (such as a back_inserter)
            void try_take(const Piece* piece, const Position& to, std::vector<Action>& actions) const;
            void possible_piece_moves(const Piece* piece, std::vector<Action>& actions) const;
            void pawn_move_with_promotions(const Piece* piece, const Position& to, std::vector<Action>& actions) const;
            void possible_pawn_moves(const Piece* piece, std::vector<Action>& actions) const;
            void line_moves(const Piece* piece, const Position& delta, std::vector<Action>& actions) const;
            void possible_bishop_moves(const Piece* piece, std::vector<Action>& actions) const;
            void possible_rook_moves(const Piece* piece, std::vector<Action>& actions) const;
            void possible_knight_moves(const Piece* piece, std::vector<Action>& actions) const;
            void possible_king_moves(const Piece* piece, std::vector<Action>& actions) const;

            // Convert to SimpleSmallState
            SimpleSmallState to_simple() const;

            // For testing
            bool operator==(const State& rhs) const;
            void print_debug_info(std::ostream& out) const;
    };
}

std::ostream& operator<<(std::ostream& out, const Skaia::State& state);


