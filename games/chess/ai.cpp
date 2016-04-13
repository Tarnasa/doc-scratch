// This is where you build your AI for the Checkers game.

#include "ai.h"

#include "SkaiaTest.h"
#include "SkaiaMM.h"

#include <atomic>
#include <thread>


/// <summary>
/// This returns your AI's name to the game server. Just replace the string.
/// </summary>
/// <returns>string of you AI's name.</returns>
std::string Chess::AI::getName()
{
    return "Petty Officer Applescab"; // REPLACE THIS WITH YOUR TEAM NAME!
}

/// <summary>
/// This is automatically called when the game first starts, once the Game object and all GameObjects have been initialized, but before any players do anything.
/// </summary>
void Chess::AI::start()
{
    // This is a good place to initialize any variables you add to your AI, or start tracking game objects.
    // Run the tests!
    SkaiaTest();
    // Initialize the average_times to somewhat meaningfull values
    // NOTE: These are intentionaly underestimates, so that, if given the chance, the AI may actually increase it's depth.
    auto make_time = [&](double time) {
        return std::chrono::nanoseconds(static_cast<uint64_t>(time * 1e9));
    };
    average_time[0] = make_time(0);
    average_time[1] = make_time(0.01);
    average_time[2] = make_time(0.5);
    average_time[3] = make_time(5.0);
    average_time[4] = make_time(10.0);
    average_time[5] = make_time(20.0);
    average_time[6] = make_time(40.0);
    average_time[7] = make_time(80.0);
    average_time[8] = make_time(160.0);
    average_time[9] = make_time(320.0);
    average_time[10] = make_time(1024.0);
    std::cout << "avg: " << average_time[1].count() << " and " << average_time[2].count() << std::endl;
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
    */

    // Record time
    auto genesis = std::chrono::steady_clock::now();

    // Print how much time since the end of our last turn
    if (state.turn > 1)
    {
        std::chrono::duration<double> slumber = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - turn_end);
        std::cout << "Slept for " << slumber.count() << " seconds." << std::endl;
    }

    // Print how much time remaining this AI has to calculate moves
    std::cout << "Time Remaining: " << this->player->timeRemaining << " ns" << std::endl;

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

    state.print_debug_info(std::cout);
    std::cout << state << std::endl;

    // Try to keep our timeRemaining higher than our opponent
    std::cout << "Difference in player time: " << this->player->timeRemaining - this->player->otherPlayer->timeRemaining << std::endl;
    auto player_time_difference = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::nanoseconds(static_cast<int64_t>(this->player->timeRemaining - this->player->otherPlayer->timeRemaining)));
    std::cout << "Time to spend: " << player_time_difference.count() << std::endl;

    // Minimax call for thread
    auto minimax = [&](int &depth, std::atomic<bool>& cont, Skaia::MMReturn& ret, std::atomic_flag &return_ready) -> void {
        while (cont)
        {
            auto action = Skaia::minimax(state, (state.turn % 2 ? Skaia::Black : Skaia::White), depth,
                std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
            while (return_ready.test_and_set()) {if (!cont) break;}
            ret = action;
            return_ready.clear();
            depth += 1;
        }
    };

    // Figure initial depth
    int depth = 2;
    if (state.turn == 0) depth = 4;

    // Get a basic action in case thread messes up somehow
    auto ret = Skaia::minimax(state, (state.turn % 2 ? Skaia::Black : Skaia::White), depth,
            std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
    std::atomic<bool> cont(true);
    std::atomic_flag action_ready;

    depth += 1;

    // Call minimax in a separate thread
    std::thread minimax_thread([&] {
        while (cont)
        {
            auto action = Skaia::minimax(state, (state.turn % 2 ? Skaia::Black : Skaia::White), depth,
                std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
            while (action_ready.test_and_set()) {if (!cont) break;}
            ret = action;
            action_ready.clear();
            depth += 1;
        }
    });

    // Wait slightly less than our opponent
    auto time_to_spend = player_time_difference - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - genesis);
    std::cout << "time_to_spend: " << time_to_spend.count() << std::endl;
    if (time_to_spend.count() > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(time_to_spend));
    }
    cont = false;
    while (action_ready.test_and_set());

    // Check time
    std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - genesis);

    std::cout << "Depth: " << depth << std::endl;
    std::cout << "Took " << duration.count() << " seconds for " << ret.states_evaluated << " states" << std::endl;
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

    action_ready.clear();
    minimax_thread.join();

    turn_end = std::chrono::steady_clock::now();

    return true; // to signify we are done with our turn.
}

