// Unit tests for the Skaia chess framework

#include "SkaiaTest.h"

#include "SkaiaBackAction.h"

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
    std::cout << sign(-2) << " " << sign(5) << " " << sign(0) << std::endl;
    std::cout << Position(5, 5) - Position(0, 0) << std::endl;
    std::cout << Position(0, 0).direction_to(Position(5, 5)) << std::endl;
    Skaia::State state;

    std::cout << "material: " << state.material(White) << " " << state.material(Black) << std::endl;

    Skaia::Piece* q = nullptr;
    Skaia::Piece* Q = nullptr;
    // Remove all pieces
    for (auto& piece : state.pieces)
    {
        if (piece.type == Skaia::Queen && piece.color == Skaia::White) {q = &piece; continue;}
        if (piece.type == Skaia::Queen && piece.color == Skaia::Black) {Q = &piece; continue;}
        state.kill_piece(&piece);
    }

    if (q == nullptr) std::cerr << "White queen not found!" << std::endl;
    print_checks(state, q->id);
    state.move_piece(q->pos, Skaia::Position(0, 0));
    print_checks(state, q->id);
    state.move_piece(Q->pos, Skaia::Position(3, 3));
    print_checks(state, q->id);
    print_checks(state, Q->id);
    state.apply_action(Skaia::Action(Q->pos, Q->pos + Position(-1, -1), Skaia::Empty));
    print_checks(state, q->id);
    print_checks(state, Q->id);

    std::cout << "material: " << state.material(White) << " " << state.material(Black) << std::endl;

    std::cout << "Testing copy constructor ";
    std::cout << (state == State(state)) << std::endl;

    std::cout << "Testing normal move back action ";
    State a, b;
    Piece empty_piece;
    SimpleSmallState null_state{{0, 0, 0, 0}};
    BackAction back_action{Empty, a.pieces[16], empty_piece, null_state, -1, a.since_pawn_or_capture};
    b.apply_action(Action(a.pieces[16].pos, a.pieces[16].pos + Position(-2, 0), Empty));
    b.apply_back_action(back_action);
    std::cout << (a == b) << std::endl;
    std::cout << "Testing cout of state: " << a << std::endl;
}

