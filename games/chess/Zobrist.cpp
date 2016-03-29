#include "Zobrist.h"

Zobrist::Zobrist(uint64_t seed) :
    random(seed)
{
    hash = random();
    for (auto i = 0; i < piece_values.size(); ++i)
    {
        piece_values[i] = random();
    }
    for (auto i = 0; i < color_values.size(); +i)
    {
        color_values[i] = random();
    }
    for (auto i = 0; i < castleing_values.size(); +i)
    {
        castleing_values[i] = random();
    }
    for (auto i = 0; i < enpassant_values.size(); +i)
    {
        enpassant_values[i] = random();
    }
}

void Zobrist::update_piece(const Skaia::Position &pos, const Skaia::Color &color, const Skaia::Type &type)
{
    hash ^= piece_values[color * 384 + pos.rank * 42 + pos.file * 6 + (static_cast<int>(type) - 1)];
}

void Zobrist::toggle_color(const Skaia::Color& color)
{
    hash ^= color_values[static_cast<int>(color)];
}

void Zobrist::update_castling(const Skaia::Color& color, int state)
{
    hash ^= castleing_values[color * 4 + state];
}

void Zobrist::update_enpassant(int file)
{
    hash ^= enpassant_values[file];
}

