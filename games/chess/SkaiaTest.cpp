// Unit tests for the Skaia chess framework

#include "SkaiaTest.h"

void print_checks(const Skaia::State& state, int piece_id)
{
    for (int rank = 0; rank < 8; ++rank)
    {
        for (int file = 0; file < 8; ++file)
        {
            auto& square = state.at(rank, file);
            char symbol = '.';
            if (square.checks[piece_id]) symbol = 'c';
            std::cout << symbol << " ";
        }
        std::cout << std::endl;
    }
}

using namespace Skaia;

void SkaiaTest()
{
    Skaia::State state;

    Skaia::Piece* q = nullptr;
    Skaia::Piece* Q = nullptr;
    // Remove all pieces
    for (auto& piece : state.pieces)
    {
        state.remove_piece(&piece);
        if (piece.type == Skaia::Queen && piece.color == Skaia::White) q = &piece;
        if (piece.type == Skaia::Queen && piece.color == Skaia::Black) Q = &piece;
    }

    print_checks(state, q->id);
    state.place_piece(q, Skaia::Position(0, 0));
    print_checks(state, q->id);
    state.place_piece(Q, Skaia::Position(3, 3));
    print_checks(state, q->id);
    print_checks(state, Q->id);
    state.apply_action(Skaia::Action(Q->pos, Q->pos + Position(-1, -1), Skaia::Empty));
    print_checks(state, q->id);
    print_checks(state, Q->id);
}

