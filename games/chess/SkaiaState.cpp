#include "SkaiaState.h"

#include <algorithm>
#include <iterator>
#include <set>

#if 1
#define LOG(s) std::cout<<s<<std::endl
#else
#define LOG(s)
#endif

namespace Skaia
{
    State::State() : turn(0), pieces(), squares(),
        pieces_by_color_and_type(), double_moved_pawn(nullptr)
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
        pieces_by_color_and_type(), double_moved_pawn(nullptr)
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
        // Copy pieces_by
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
    }

    void State::move_piece(const Position& from, const Position& to)
    {
        Piece* piece = at(from).piece;
        remove_piece(piece);
        place_piece(piece, to);
    }

    void State::apply_action(const Action& action)
    {
        LOG("apply_action");
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
                    to.piece->alive = false;
                    remove_piece(to.piece);
                }
                Piece* piece = from.piece;
                move_piece(action.from, action.to);
                remove_piece(piece);
                auto& v = pieces_by_color_and_type[piece->color][piece->type];
                v.erase(std::find(v.begin(), v.end(), piece));
                piece->type = action.promotion;
                pieces_by_color_and_type[piece->color][piece->type].emplace_back(piece);
                place_piece(piece, piece->pos);
            }
            // Castling
            else if (from.piece->type == King && std::abs(static_cast<int>(action.from.file - action.to.file)) == 2)
            {
                LOG("Castling");
                at(action.from).piece->special = false;
                move_piece(action.from, action.to);
                Position rook_from(action.from.rank, action.to.file == 2 ? 0 : 7);
                Position rook_to(action.from.rank, action.to.file == 2 ? 3 : 5);
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
                }
                else // En passant
                {
                    LOG("En passant");
                    move_piece(action.from, action.to);
                    Piece* piece = at(action.from.rank, action.to.file).piece;
                    kill_piece(piece);
                }
            }
        }
        // Normal move
        else
        {
            if (to.piece != nullptr)
            {
                kill_piece(to.piece);
            }
            move_piece(action.from, action.to);
            to.piece->special = false;
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
    }

    std::vector<Action> State::generate_actions() const
    {
        LOG("generate_actions");
        std::vector<Action> actions;
        for (auto&& piece : pieces)
        {
            if (piece.alive && piece.color == turn % 2)
            {
                possible_piece_moves(&piece, actions);
            }
        }
        std::vector<Action> actions2;
        auto inserter = std::back_inserter(actions2);
        for (auto&& piece : pieces)
        {
            if (piece.alive && piece.color == turn % 2)
            {
                std::copy(piece.moves.begin(), piece.moves.end(), inserter);
            }
        }
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
            actions.emplace_back(piece->pos, to, Bishop);
            actions.emplace_back(piece->pos, to, Knight);
            actions.emplace_back(piece->pos, to, Rook);
            actions.emplace_back(piece->pos, to, Queen);
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
        if (piece->pos.rank == (piece->color ? 1 : 6))
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
}

std::ostream& operator<<(std::ostream& out, const Skaia::State& state)
{
    static const std::string type_chars = ">pbnrqk";
    for (int rank = 0; rank < 8; ++rank)
    {
        for (int file = 0; file < 8; ++file)
        {
            Skaia::Piece* piece = state.at(rank, file).piece;
            char symbol = type_chars[piece->type];
            if (piece->color) symbol -= ('a' - 'A');
            out << symbol << " ";
        }
        out << std::endl;
    }
    return out;
}

#undef LOG

