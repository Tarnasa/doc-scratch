#pragma once

// Zobrist is a hash for board states.
// It can be incrementally updated (and un-updated) as actions
//  are taken and untaken.

#include <cstdint>
#include <random>
#include <array>

#include "Skaia.h"

class Zobrist
{
    public:
        uint64_t hash;

        // An array of values which correspond to a given type and a given position
        std::array<uint64_t, 2 * 6 * 8 * 8> piece_values;
        std::array<uint64_t, 2> color_values; // [0] for White, [1] for Black
        std::array<uint64_t, 8> castleing_values; // 0/1/2/3 for White castle None/King/Queen/Both side
        std::array<uint64_t, 8> enpassant_values; // One for each file



        Zobrist(uint64_t seed);
        // Call these functions a second time to undo the first
        void update_piece(const Skaia::Position &pos, const Skaia::Color &color, const Skaia::Type &type);
        void toggle_color(const Skaia::Color& color);
        void update_castling(const Skaia::Color& color, int state);
        void update_enpassant(int file);
    private:
        std::mt19937_64 random;
};

