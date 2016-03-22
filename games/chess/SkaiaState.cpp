#include "SkaiaState.h"

#include <algorithm>
#include <iterator>
#include <set>

#if 0
#define LOG(s) std::cout<<s<<std::endl
#else
#define LOG(s)
#endif

namespace Skaia
{
    State::State() : turn(0), pieces(), squares(),
        pieces_by_color_and_type(), double_moved_pawn(nullptr), history(7),
        since_pawn_or_capture(0)
    {
        // Generate pieces
        static const std::vector<Type> order = {Rook, Knight, Bishop, Queen, King, Bishop, Knight, Rook};
        auto create_piece = [this](const Piece& piece) {
            pieces[piece.id] = piece;
            Piece* ptr = &(pieces[piece.id]);
            at(piece.pos).piece = ptr;
            pieces_by_color_and_type[piece.color][piece.type].emplace_back(ptr);
        };
        for (auto file = 0; file < 8; ++file)
        {
            // Home row
            create_piece(Piece(Position(0, file), order[file], Black, file + 0*8, true));
            create_piece(Piece(Position(7, file), order[file], White, file + 3*8, true));
            // Pawns
            create_piece(Piece(Position(1, file), Pawn, Black, file + 1*8, true));
            create_piece(Piece(Position(6, file), Pawn, White, file + 2*8, true));
        }
        // Generate checks
        for (auto&& piece : pieces)
        {
            place_piece(&piece, piece.pos);
        }
    }

    State::State(const State& source) : turn(source.turn), pieces(source.pieces), squares(),
        pieces_by_color_and_type(), double_moved_pawn(nullptr), history(source.history),
        since_pawn_or_capture(source.since_pawn_or_capture)
    {
        auto make_pointer = [&, this](const Piece* piece) {
            return piece == nullptr ? nullptr : &(this->pieces[piece->id]);
        };
        // Copy squares
        for (auto i = 0; i < squares.size(); ++i)
        {
            squares[i].piece = make_pointer(source.squares[i].piece);
            squares[i].checks = source.squares[i].checks;
        }
        // Copy pieces_by_color_and_type
        for (auto color = 0; color < 2; ++color)
        {
            for (auto type = 0; type < NumberOfTypes; ++type)
            {
                auto& sp = source.pieces_by_color_and_type[color][type];
                std::transform(sp.cbegin(), sp.cend(),
                        std::back_inserter(pieces_by_color_and_type[color][type]),
                        [&](const Piece* piece) {
                            return make_pointer(piece);
                        });
            }
        }
        // Copy double pawn
        double_moved_pawn = make_pointer(source.double_moved_pawn);
    }

    bool State::draw() const
    {
        if (since_pawn_or_capture ==50) return true;
        if (history.size() == 7)
        {
            return
                history[0] == history[4] &&
                history[1] == history[5] &&
                history[2] == history[6] &&
                history[3] == to_simple();
        }
        return false;
    }


    void State::place_piece(Piece* piece, const Position& pos)
    {
        LOG("place_piece");
        // Always assume that target position is empty
        // Check for blocking other piece's lines
        typedef std::pair<Piece*, Position> UpdatePair;
        std::vector<UpdatePair> need_updating;
        if (at(pos).checks.any())
        {
            auto& checks = at(pos).checks;
            for (int i = 0; i < checks.size(); ++i)
            {
                if (checks[i])
                {
                    auto& attacker = pieces[i];
                    switch (attacker.type)
                    {
                        case Bishop:
                        case Rook:
                        case Queen:
                            check_ray(&attacker, attacker.pos.direction_to(pos), false);
                            need_updating.emplace_back(&attacker, attacker.pos.direction_to(pos));
                            break;
                    }
                }
            }
        }
        // Place piece at location
        piece->pos = pos;
        at(pos).piece = piece;
        // Update possible moves
        if (at(pos).checks.any())
        {
            auto& checks = at(pos).checks;
            // TODO: Optimize with CLZ instruction
            for (int i = 0; i < checks.size(); ++i)
            {
                if (checks[i])
                {
                    auto& attacker = pieces[i];
                    attacker.moves.clear();
                    possible_piece_moves(&attacker, attacker.moves);
                }
            }
        }
        // Update sight lines and moves for other pieces
        for (auto i : need_updating)
        {
            check_ray(i.first, i.second, true);
            i.first->moves.clear();
            possible_piece_moves(i.first, i.first->moves);
        }
        // Update nearby pawns
        for (auto& pos : std::vector<Position>{{-2, 0}, {-1, 0}, {1, 0}, {2, 0}})
        {
            Position new_pos = piece->pos + pos;
            if (inside(new_pos) && at(new_pos).piece != nullptr)
            {
                Piece* piece = at(new_pos).piece;
                piece->moves.clear();
                possible_piece_moves(piece, piece->moves);
            }
        }
        // Update sight lines and moves for this piece
        check_piece(piece, true);
        piece->moves.clear();
        possible_piece_moves(piece, piece->moves);
    }

    void State::remove_piece(Piece* piece)
    {
        LOG("remove_piece");
        // Remove own checks and moves
        check_piece(piece, false);
        piece->moves.clear();

        // Remove from board
        at(piece->pos).piece = nullptr;
        // Update nearby pawns
        for (auto& pos : std::vector<Position>{{-2, 0}, {-1, 0}, {1, 0}, {2, 0}})
        {
            Position new_pos = piece->pos + pos;
            if (inside(new_pos) && at(new_pos).piece != nullptr)
            {
                Piece* piece = at(new_pos).piece;
                piece->moves.clear();
                possible_piece_moves(piece, piece->moves);
            }
        }
        // Check for blocking other pieces
        if (at(piece->pos).checks.any())
        {
            auto& checks = at(piece->pos).checks;
            for (int i = 0; i < checks.size(); ++i)
            {
                if (checks[i])
                {
                    auto& attacker = pieces[i];
                    // Update moves
                    attacker.moves.clear();
                    possible_piece_moves(&attacker, attacker.moves);
                    // Update ray checks
                    switch (attacker.type)
                    {
                        case Bishop:
                        case Rook:
                        case Queen:
                            check_ray(&attacker, attacker.pos.direction_to(piece->pos), true);
                            break;
                    }
                }
            }
        }
    }

    void State::kill_piece(Piece* piece)
    {
        LOG("kill_piece");
        remove_piece(piece);
        // Set dead and remove from pieces_by_color_and_type
        piece->alive = false;
        auto& v = pieces_by_color_and_type[piece->color][piece->type];
        v.erase(std::find(v.begin(), v.end(), piece));
        since_pawn_or_capture = 0;
    }

    void State::move_piece(const Position& from, const Position& to)
    {
        Piece* piece = at(from).piece;
        remove_piece(piece);
        place_piece(piece, to);
    }

    BackAction State::apply_action(const Action& action)
    {
        LOG("apply_action");
        // Create a BackAction so that we can return to this state
        Piece null_piece;
        SimpleSmallState old_state(history.size() == 7 ? history[0] :
                SimpleSmallState{{{0, 0, 0, 0}}});
        int double_moved_id = double_moved_pawn == nullptr ? -1 : double_moved_pawn->id;
        BackAction back_action{action.promotion, Piece(*(at(action.from).piece)),
            null_piece, old_state, double_moved_id, since_pawn_or_capture};

        // Record this state so we can check for draws later
        history.push_back(to_simple());
        since_pawn_or_capture += 1;

        Piece* new_double_moved_pawn = nullptr;

        State::Square& from = at(action.from);
        State::Square& to = at(action.to);
        // Special cases
        if (action.promotion != Empty)
        {
            // Promotion
            if (from.piece->type == Pawn && action.to.rank == (from.piece->color == Black ? 7 : 0))
            {
                LOG("Promotion");
                if (to.piece != nullptr)
                {
                    back_action.taken = *(to.piece);
                    kill_piece(to.piece);
                }
                Piece* piece = from.piece;
                move_piece(action.from, action.to);
                remove_piece(piece);
                auto& v = pieces_by_color_and_type[piece->color][piece->type];
                v.erase(std::find(v.begin(), v.end(), piece));
                piece->type = action.promotion;
                pieces_by_color_and_type[piece->color][piece->type].emplace_back(piece);
                place_piece(piece, piece->pos);
                since_pawn_or_capture = 0;
            }
            // Castling
            else if (from.piece->type == King && std::abs(static_cast<int>(action.from.file - action.to.file)) == 2)
            {
                LOG("Castling");
                at(action.from).piece->special = false;
                move_piece(action.from, action.to);
                Position rook_from(action.from.rank, action.to.file == 2 ? 0 : 7);
                Position rook_to(action.from.rank, action.to.file == 2 ? 3 : 5);
                // Record old rook state
                back_action.taken = *(at(rook_from).piece);
                // Move rook
                at(rook_from).piece->special = false;
                move_piece(rook_from, rook_to);
            }
            // En passant and double move
            else if (from.piece->type == Pawn)
            {
                if (action.promotion == Pawn) // Double move
                {
                    move_piece(action.from, action.to);
                    new_double_moved_pawn = at(action.to).piece;
                    at(action.to).piece->special = false;
                    since_pawn_or_capture = 0;
                }
                else // En passant
                {
                    LOG("En passant");
                    move_piece(action.from, action.to);
                    // Record killed pawn
                    Piece* piece = at(action.from.rank, action.to.file).piece;
                    back_action.taken = *piece;
                    // Kill pawn
                    kill_piece(piece);
                }
            }
        }
        // Normal move
        else
        {
            if (to.piece != nullptr)
            {
                back_action.taken = *(to.piece);
                kill_piece(to.piece);
            }
            move_piece(action.from, action.to);
            to.piece->special = false;
            if (to.piece->type == Pawn)
            {
                since_pawn_or_capture = 0;
            }
        }

        // Remove moves to en-passant the previously double-moved pawn
        if (double_moved_pawn != nullptr)
        {
            Position adjacent = double_moved_pawn->pos + Position(0, 1);
            // Update double_moved_pawn so that this actually removes moves
            double_moved_pawn = new_double_moved_pawn;
            // Remove moves
            if (inside(adjacent) && at(adjacent).piece != nullptr)
            {
                // TODO: Combine possible_piece_moves and clear()
                at(adjacent).piece->moves.clear();
                possible_piece_moves(at(adjacent).piece, at(adjacent).piece->moves);
            }
            adjacent += Position(0, -2);
            if (inside(adjacent) && at(adjacent).piece != nullptr)
            {
                at(adjacent).piece->moves.clear();
                possible_piece_moves(at(adjacent).piece, at(adjacent).piece->moves);
            }
        }
        // Update double_moved_pawn so that moves are added
        double_moved_pawn = new_double_moved_pawn;
        // Add the moves to en-passant the currently double-moved pawn
        if (double_moved_pawn != nullptr)
        {
            Position adjacent = double_moved_pawn->pos + Position(0, 1);
            if (inside(adjacent) && at(adjacent).piece != nullptr)
            {
                at(adjacent).piece->moves.clear();
                possible_piece_moves(at(adjacent).piece, at(adjacent).piece->moves);
            }
            adjacent += Position(0, -2);
            if (inside(adjacent) && at(adjacent).piece != nullptr)
            {
                at(adjacent).piece->moves.clear();
                possible_piece_moves(at(adjacent).piece, at(adjacent).piece->moves);
            }
        }

        turn += 1;
        return back_action;
    }

    void State::apply_back_action(const BackAction& action)
    {
        LOG("apply_back_action");
        Piece* old_double_moved_pawn = (action.double_moved_pawn_id == -1 ? nullptr :
                &(pieces[action.double_moved_pawn_id]));

        // Remove the moves to en-passant the currently double-moved pawn
        if (double_moved_pawn != nullptr)
        {
            Position adjacent = double_moved_pawn->pos + Position(0, 1);
            // Update double_moved_pawn so that this actually removes moves
            double_moved_pawn = old_double_moved_pawn;
            // Remove moves
            if (inside(adjacent) && at(adjacent).piece != nullptr)
            {
                at(adjacent).piece->moves.clear();
                possible_piece_moves(at(adjacent).piece, at(adjacent).piece->moves);
            }
            adjacent += Position(0, -2);
            if (inside(adjacent) && at(adjacent).piece != nullptr)
            {
                at(adjacent).piece->moves.clear();
                possible_piece_moves(at(adjacent).piece, at(adjacent).piece->moves);
            }
        }
        // Update double_moved_pawn so that moves are added
        double_moved_pawn = old_double_moved_pawn;
        // Add the moves to en-passant the previously double-moved pawn
        if (double_moved_pawn != nullptr)
        {
            Position adjacent = double_moved_pawn->pos + Position(0, 1);
            if (inside(adjacent) && at(adjacent).piece != nullptr)
            {
                at(adjacent).piece->moves.clear();
                possible_piece_moves(at(adjacent).piece, at(adjacent).piece->moves);
            }
            adjacent += Position(0, -2);
            if (inside(adjacent) && at(adjacent).piece != nullptr)
            {
                at(adjacent).piece->moves.clear();
                possible_piece_moves(at(adjacent).piece, at(adjacent).piece->moves);
            }
        }

        Piece* piece = &(pieces[action.actor.id]);

        // Move attacking piece back
        if (action.type != Empty)
        {
            // Promotion
            if (action.actor.type != piece->type)
            {
                remove_piece(piece);

                auto& v = pieces_by_color_and_type[piece->color][piece->type];
                v.erase(std::find(v.begin(), v.end(), piece));
                *piece = action.actor;
                pieces_by_color_and_type[piece->color][piece->type].emplace_back(piece);

                if (action.taken.id != -1) // If a piece was taken
                {
                    Piece* taken = &(pieces[action.taken.id]);
                    *taken = action.taken;
                    pieces_by_color_and_type[taken->color][taken->type].emplace_back(taken);
                    place_piece(taken, taken->pos);
                }
                place_piece(piece, piece->pos);
            }
            // Castleing
            else if (action.type == King && piece->type == King)
            {
                // Return king
                remove_piece(piece);
                *piece = action.actor;
                place_piece(piece, piece->pos);
                // Return rook
                Piece* taken = &(pieces[action.taken.id]);
                remove_piece(taken);
                *taken = action.taken;
                place_piece(taken, taken->pos);
            }
            // En passant and double move
            else if (action.actor.type == Pawn)
            {
                if (action.type == Pawn) // Double move
                {
                    remove_piece(piece);
                    *piece = action.actor;
                    place_piece(piece, piece->pos);
                }
                else // En passant
                {
                    remove_piece(piece);
                    *piece = action.actor;
                    place_piece(piece, piece->pos);
                    // Restore taken pawn
                    Piece* taken = &(pieces[action.taken.id]);
                    *taken = action.taken;
                    pieces_by_color_and_type[taken->color][taken->type].emplace_back(taken);
                    place_piece(taken, taken->pos);
                }
            }
        }
        else
        // Normal move
        {
            remove_piece(piece);
            *piece = action.actor;
            if (action.taken.id != -1) // If a piece was taken
            {
                Piece* taken = &(pieces[action.taken.id]);
                *taken = action.taken;
                pieces_by_color_and_type[taken->color][taken->type].emplace_back(taken);
                place_piece(taken, taken->pos);
            }
            place_piece(piece, piece->pos);
        }

        // Restore state variables
        since_pawn_or_capture = action.since_pawn_or_capture;
        SimpleSmallState null_state{{0, 0, 0, 0}};
        // Add old state
        if (action.old_state != null_state)
        {
            history.push_front(action.old_state);
        }
        else // Remove a state, we are near the beginning
        {
            history.pop_back();
        }
        turn -= 1;
    }

    std::vector<Action> State::generate_actions() const
    {
        LOG("generate_actions");
        /*
        std::vector<Action> actions;
        for (auto&& piece : pieces)
        {
            if (piece.alive && piece.color == turn % 2)
            {
                possible_piece_moves(&piece, actions);
            }
        }
        */
        std::vector<Action> actions;
        auto inserter = std::back_inserter(actions);
        for (auto&& piece : pieces)
        {
            if (piece.alive && piece.color == turn % 2)
            {
                std::copy(piece.moves.begin(), piece.moves.end(), inserter);
            }
        }
        /*
        std::set<Action> a(actions.begin(), actions.end());
        std::set<Action> b(actions2.begin(), actions2.end());
        if (a != b)
        {
            std::cerr << "Moves not the same" << std::endl;
            for (auto&& action : actions)
            {
                std::cerr << action << std::endl;
            }
            std::cerr << "actions2:" << std::endl;
            for (auto&& action : actions2)
            {
                std::cerr << action << std::endl;
            }
            std::cerr << "State: " << *this;
        }
        */

        // Remove actions which would put the moving player into check
        std::vector<Action> safe_actions;
        std::remove_copy_if(actions.begin(), actions.end(), std::back_inserter(safe_actions),
                [this](const Action& action){
                    State new_state(*this);
                    new_state.apply_action(action);
                    return new_state.is_in_check(this->turn % 2 ? Black : White);
                });
        return safe_actions;
    }

    bool State::is_in_check(Color color) const
    {
        return at(pieces_by_color_and_type[color][King][0]->pos).checked_by_color(!color ? Black : White);
    }

    int State::material(Color color) const
    {
        static const std::array<std::pair<Type, int>, 6> types = {{
            {Pawn, 1},
            {Bishop, 3},
            {Knight, 3},
            {Rook, 5},
            {Queen, 9},
            {King, 0}
        }};
        int total_value = 0;
        for (auto& type : types)
        {
            total_value += type.second * pieces_by_color_and_type[color][type.first].size();
        }
        return total_value;
    }

    int State::count_net_checks(Color color) const
    {
        // TODO: Cache all dis
        static const std::array<std::pair<Type, int>, 6> protect_value = {{
            {Pawn, 1},
            {Bishop, 3},
            {Knight, 3},
            {Rook, 5},
            {Queen, 9},
            {King, 0},
        }};
        /*
        static const std::map<Type, int> check_value = {
            {Pawn, 6},
            {Bishop, 3},
            {Knight, 3},
            {Rook, 3},
            {Queen, 2},
            {King, 1},
        };
        */
        auto count = [&](const Square& square, Color color) {
            if (color == Black)
            {
                return (square.checks >> 16).count();
            }
            else
            {
                return (square.checks << 16).count();
            }
        };
        int checks = 0;
        for (auto& type : protect_value)
        {
            for (Piece* piece : pieces_by_color_and_type[color][type.first])
            {
                checks += count(at(piece->pos), color) - count(at(piece->pos), !color ? Black : White);
            }
        }
        return checks;
    }

    SimpleSmallState State::to_simple() const
    {
        auto convert_piece = [&](const Piece* piece) {
            return piece == nullptr ? 0 : (
                    static_cast<uint64_t>(piece->type) +
                    (static_cast<uint64_t>(piece->color) << 3));
        };
        auto convert_two_rows = [&](int start) -> uint64_t {
            return
                (convert_piece(squares[start++].piece) << 0 ) +
                (convert_piece(squares[start++].piece) << 4 ) +
                (convert_piece(squares[start++].piece) << 8 ) +
                (convert_piece(squares[start++].piece) << 12) +
                (convert_piece(squares[start++].piece) << 16) +
                (convert_piece(squares[start++].piece) << 20) +
                (convert_piece(squares[start++].piece) << 24) +
                (convert_piece(squares[start++].piece) << 28) +
                (convert_piece(squares[start++].piece) << 32) +
                (convert_piece(squares[start++].piece) << 36) +
                (convert_piece(squares[start++].piece) << 40) +
                (convert_piece(squares[start++].piece) << 44) +
                (convert_piece(squares[start++].piece) << 48) +
                (convert_piece(squares[start++].piece) << 52) +
                (convert_piece(squares[start++].piece) << 56) +
                (convert_piece(squares[start++].piece) << 60);
        };
        return SimpleSmallState{{{convert_two_rows(0),
            convert_two_rows(16),
            convert_two_rows(32),
            convert_two_rows(48),
        }}};
    }

    // Testing functions
    bool State::operator==(const State& rhs) const
    {
        auto check = [](bool cond, const std::string& message) {
            if (!cond)
            {
                std::cerr << "Assertion: " << message << " failed" << std::endl;
            }
            return cond;
        };
        // Convenience function for comparing pointers which may be null
        auto cmp_ptr = [](Piece* a, Piece* b) {
            if (a == nullptr) return b == nullptr;
            if (b == nullptr) return a == nullptr;
            return a->id == b->id;
        };
        // Compare pieces_by_color_and_type
        for (Color color : std::vector<Color>{White, Black})
        {
            for (Type type : std::vector<Type>{Pawn, Bishop, Knight, Rook, Queen, King})
            {
                if (pieces_by_color_and_type[color][type].size() !=
                        rhs.pieces_by_color_and_type[color][type].size())
                {
                    std::cerr << "Assertion: pieces_by_color_and_type.size failed" << std::endl;
                    return false;
                }
                std::set<int> lhs_set, rhs_set;
                for (int i = 0; i < pieces_by_color_and_type[color][type].size(); ++i)
                {
                    lhs_set.insert(pieces_by_color_and_type[color][type][i]->id);
                    rhs_set.insert(rhs.pieces_by_color_and_type[color][type][i]->id);
                }
                if (lhs_set != rhs_set)
                {
                    std::cerr << "Assertion: pieces_by_color_and_type failed" << std::endl;
                    return false;
                }
            }
        }
        // Compare the rest of the stuff
        return
            check(turn == rhs.turn, "turn") &&
            check(pieces == rhs.pieces, "pieces") &&
            check(squares == rhs.squares, "squares") &&
            check(cmp_ptr(double_moved_pawn, rhs.double_moved_pawn), "double") &&
            check(history == rhs.history, "history") &&
            check(since_pawn_or_capture == rhs.since_pawn_or_capture, "since");
    }

    bool State::Square::operator==(const Square& rhs) const
    {
        // Convenience function for comparing pointers which may be null
        auto cmp_ptr = [](Piece* a, Piece* b) {
            if (a == nullptr) return b == nullptr;
            if (b == nullptr) return a == nullptr;
            return a->id == b->id;
        };
        return
            cmp_ptr(piece, rhs.piece) &&
            checks == rhs.checks;
    }
}

std::ostream& operator<<(std::ostream& out, const Skaia::State& state)
{
    static const std::string type_chars = ">pbnrqk";
    for (int rank = 0; rank < 8; ++rank)
    {
        for (int file = 0; file < 8; ++file)
        {
            Skaia::Piece* piece = state.at(rank, file).piece;
            char symbol = '.';
            if (piece != nullptr)
            {
                symbol = type_chars[piece->type];
                if (piece->color) symbol -= ('a' - 'A');
            }
            out << symbol << " ";
        }
        out << std::endl;
    }
    return out;
}

#undef LOG

