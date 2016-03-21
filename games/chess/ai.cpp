// This is where you build your AI for the Checkers game.

#include "ai.h"

#include "SkaiaTest.h"
#include "SkaiaMM.h"

#include <chrono>


/// <summary>
/// This returns your AI's name to the game server. Just replace the string.
/// </summary>
/// <returns>string of you AI's name.</returns>
std::string Chess::AI::getName()
{
    return "Chief Deputy Pyralspite"; // REPLACE THIS WITH YOUR TEAM NAME!
}

/// <summary>
/// This is automatically called when the game first starts, once the Game object and all GameObjects have been initialized, but before any players do anything.
/// </summary>
void Chess::AI::start()
{
    // This is a good place to initialize any variables you add to your AI, or start tracking game objects.
}

/// <summary>
/// This is automatically called every time the game (or anything in it) updates.
/// </summary>
void Chess::AI::gameUpdated()
{
    // If a function you call triggers an update this will be called before that function returns.
}

/// <summary>
/// This is automatically called when the game ends.
/// </summary>
/// <param name="won">true if your player won, false otherwise</param>
/// <param name="reason">a string explaining why you won or lost</param>
void Chess::AI::ended(bool won, std::string reason)
{
    // You can do any cleanup of you AI here, or do custom logging. After this function returns the application will close.
}


/// <summary>
/// This is called every time it is this AI.player's turn.
/// </summary>
/// <returns>Represents if you want to end your turn. True means end your turn, False means to keep your turn going and re-call this function.</returns>
bool Chess::AI::runTurn()
{
    // Here is where you'll want to code your AI.

    // We've provided sample code that:
    //    1) prints the board to the console
    //    2) prints the opponent's last move to the console
    //    3) prints how much time remaining this AI has to calculate moves
    //    4) makes a random (and probably invalid) move.

    /*
    // 1) print the board to the console
    for (int rank = 9; rank >= -1; rank--)
    {
        std::string str = "";
        if (rank == 9 || rank == 0) // then the top or bottom of the board
        {
            str = "   +------------------------+";
        }
        else if (rank == -1) // then show the files
        {
            str = "     a  b  c  d  e  f  g  h";
        }
        else // board
        {
            str += " ";
            str += std::to_string(rank);
            str += " |";
            // fill in all the rank with pieces at the current file
            for (int file = 0; file < 8; file++)
            {
                std::string file_string(1, (char)(((int)"a"[0]) + file)); // start at a, with with rank offset increasing the char;
                Chess::Piece* currentPiece = nullptr;
                for (auto piece : this->game->pieces)
                {
                    if (piece->rank == rank && piece->file == file_string) // then we found the piece at (rank, file)
                    {
                        currentPiece = piece;
                        break;
                    }
                }

                char code = '.'; // default "no piece";
                if (currentPiece != nullptr)
                {
                    code = currentPiece->type[0];

                    if (currentPiece->type == "Knight") // 'K' is for "King", we use 'N' for "Knights"
                    {
                        code = 'N';
                    }

                    if (currentPiece->owner->id == "1") // the second player (black) is lower case. Otherwise it's upppercase already
                    {
                        code = tolower(code);
                    }
                }

                str += " ";
                str += code;
                str += " ";
            }

            str += "|";
        }

        std::cout << str << std::endl;
    }

    // 2) print the opponent's last move to the console
    if (this->game->moves.size() > 0) {
        std::cout << "Opponent's Last Move: '" << this->game->moves[this->game->moves.size() - 1]->san << "'" << std::endl;
    }

    // 3) print how much time remaining this AI has to calculate moves
    std::cout << "Time Remaining: " << this->player->timeRemaining << " ns" << std::endl;
    */

    std::cout << "Turn number: " << this->game->currentTurn << std::endl;
    // Apply previous move to state
    if (this->game->currentTurn > 0)
    {
        Move& move = *(this->game->moves.back());
        Skaia::Position from(Skaia::rank_to_skaia(move.fromRank), Skaia::file_to_skaia(move.fromFile));
        Skaia::Position to(Skaia::rank_to_skaia(move.toRank), Skaia::file_to_skaia(move.toFile));
        Skaia::Type promotion = Skaia::type_to_skaia(move.promotion);

        Skaia::Piece* piece = state.at(from).piece;
        // Detect double pawn move
        if (piece->type == Skaia::Pawn && abs(to.rank - from.rank) == 2)
        {
            promotion = Skaia::Pawn;
        }
        // Detect en-passant
        if (piece->type == Skaia::Pawn && to.file != from.file && state.at(to).piece == nullptr)
        {
            promotion = Skaia::King;
        }
        /// Detect castling
        else if (piece->type == Skaia::King && abs(from.file - to.file) == 2)
        {
            promotion = Skaia::King;
        }

        state.apply_action(Skaia::Action(from, to, promotion));
    }
    state.turn = this->game->currentTurn;

    // IDDL-MM
    int depth = 1;
    // Record time
    auto genesis = std::chrono::steady_clock::now();
    while (true)
    {
        // Minimax
        auto ret = Skaia::minimax(state, (state.turn % 2 ? Skaia::Black : Skaia::White), depth);

        // Check time
        std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - genesis);
        // Go deeper if we haven't reached 2 seconds
        if (duration.count() < 2.0)
        {
            depth += 1;
            continue;
        }
        std::cout << "Took " << duration.count() << " seconds for depth=" << depth << std::endl;
        std::cout << "Heuristic " << ret.heuristic << " with action " << ret.action << std::endl;
        
        // Make move through framework
        auto move = ret.action;
        auto from_rank = Skaia::rank_from_skaia(move.from.rank);
        auto from_file = Skaia::file_from_skaia(move.from.file);
        auto to_rank = Skaia::rank_from_skaia(move.to.rank);
        auto to_file = Skaia::file_from_skaia(move.to.file);
        for (auto&& piece : this->player->pieces)
        {
            if (piece->rank == from_rank && piece->file == from_file)
            {
                piece->move(to_file, to_rank, Skaia::type_from_skaia(move.promotion));
            }
        }
        // Apply move to state
        state.apply_action(move);

        break;
    }

    return true; // to signify we are done with our turn.
}

