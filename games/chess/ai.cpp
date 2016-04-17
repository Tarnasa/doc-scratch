// This is where you build your AI for the Checkers game.

#include "ai.h"

#include "SkaiaTest.h"

#include <atomic>
#include <thread>


/// <summary>
/// This returns your AI's name to the game server. Just replace the string.
/// </summary>
/// <returns>string of you AI's name.</returns>
std::string Chess::AI::getName()
{
    return "Spades Slick"; // REPLACE THIS WITH YOUR TEAM NAME!
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
    std::cout << "Waiting for threads to finish" << std::endl;
    if (!idmm_stop)
    {
        idmm_stop = true;
        idmm_thread.join();
    }
    if (!pondering_stop)
    {
        pondering_stop = true;
        pondering_thread.join();
    }
    std::cout << "Threads finished" << std::endl;
}


/// <summary>
/// This is called every time it is this AI.player's turn.
/// </summary>
/// <returns>Represents if you want to end your turn. True means end your turn, False means to keep your turn going and re-call this function.</returns>
bool Chess::AI::runTurn()
{
    // Here is where you'll want to code your AI.

    // Shorthands so we don't get too verbose
    using std::chrono::nanoseconds;
    using std::chrono::milliseconds;
    using std::chrono::seconds;
    using std::chrono::duration_cast;

    std::cout << "begin" << std::endl; // TODO: Remove
    // Record time
    auto genesis = std::chrono::steady_clock::now();

    // Print how much time since the end of our last turn
    if (state.turn > 1)
    {
        std::chrono::duration<double> slumber = duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - turn_end);
        std::cout << "Slept for " << slumber.count() << " seconds." << std::endl;
    }

    if (this->game->currentTurn > 1)
    {
        // Wait for the pondering thread to finish
        std::cout << "joining pondering thread" << std::endl; // TODO: Remove
        pondering_stop = true;
        pondering_thread.join();
        std::cout << "joined pondering thread" << std::endl; // TODO: Remove
    }

    // Keep track of the previous action
    Skaia::Action previous_action(Skaia::Position(-1, -1), Skaia::Position(-1, -1), Skaia::Pawn);

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

        previous_action = Skaia::Action(from, to, promotion);
        state.apply_action(previous_action);
    }
    state.turn = this->game->currentTurn;

    // Print the current state
    state.print_debug_info(std::cout);
    std::cout << state << std::endl;

    // Print how much time remaining this AI has to calculate moves
    std::cout << "Turn number: " << this->game->currentTurn << std::endl;
    std::cout << "Time Remaining: " << this->player->timeRemaining << " ns" << std::endl;

    // Find the difference in remaining time between players
    std::cout << "Difference in player time: " << this->player->timeRemaining - this->player->otherPlayer->timeRemaining << std::endl;
    auto player_time_difference = duration_cast<milliseconds>(
            nanoseconds(static_cast<int64_t>(this->player->timeRemaining - this->player->otherPlayer->timeRemaining)));
    std::cout << "Time to spend: " << player_time_difference.count() << std::endl;

    // Determine the depth we need to start search at
    int depth = 4;
    if (state.turn <= 1) depth = 4;

    // Try to get result of pondering thread
    Skaia::MMReturn ret{-1, previous_action, -1};
    bool found_pondering_result = false;
    if (state.turn > 1)
    {
        // Print out how far the pondering thread searched
        std::cout << "Pondering thread searched to depth of: " << pondering_depth << std::endl;
        // Use the result of the pondering thread
        for (auto &pair : pondering_move)
        {
            if (pair.first == previous_action)
            {
                ret = pair.second;
                found_pondering_result = true;
                depth = pondering_depth - 1;
                break;
            }
        }
    }
    if (!found_pondering_result)
    {
        // Generate a simple action in case the idmm_thread somehow fails
        ret = Skaia::minimax(state, (state.turn % 2 ? Skaia::Black : Skaia::White), depth,
                std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max(), history_table);
        if (state.turn > 1)
        {
            std::cerr << "Failed to find result from pondering thread!" << std::endl;
        }
        depth += 1;
    }

    std::cout << "before thread" << std::endl;
    // Call minimax in a separate thread
    std::cout << "1" << std::endl;
    idmm_stop = false;
    std::cout << "2" << std::endl;
    idmm_busy.clear();
    std::cout << "3" << std::endl;
    idmm_thread = std::thread([&] {
        Skaia::State state_copy = state;
        while (!idmm_stop)
        {
            std::cout << "4" << std::endl;
            auto action = Skaia::interruptable_minimax(state_copy,
                    (state_copy.turn % 2 ? Skaia::Black : Skaia::White),
                    depth,
                    std::numeric_limits<int>::lowest(),
                    std::numeric_limits<int>::max(),
                    history_table,
                    idmm_stop);
            std::cout << "5" << std::endl;
            while (idmm_busy.test_and_set() && !idmm_stop);
            std::cout << "6" << std::endl;
            if (idmm_stop)
            {
                idmm_busy.clear();
                break;
            }
            std::cout << "7" << std::endl;
            ret = action;
            std::cout << "8" << std::endl;
            idmm_busy.clear();
            std::cout << "9" << std::endl;
            depth += 1;
        }
    });
    std::cout << "after thread" << std::endl;

    // Wait slightly less than our opponent
    auto time_to_spend = player_time_difference -
        duration_cast<milliseconds>(std::chrono::steady_clock::now() - genesis);
    time_to_spend -= milliseconds(300);
    // But slightly more if we are losing
    if (ret.heuristic < -1000) time_to_spend += milliseconds(600);
    // And cap off our min and max time
    if (time_to_spend < seconds(1)) time_to_spend = seconds(1);
    if (time_to_spend > seconds(20)) time_to_spend = seconds(20);

    std::cout << "time_to_spend: " << time_to_spend.count() << std::endl;
    if (time_to_spend.count() > 0)
    {
        std::this_thread::sleep_for(milliseconds(time_to_spend));
    }
    idmm_stop = true;
    while (idmm_busy.test_and_set());

    // Check time
    std::chrono::duration<double> duration = duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - genesis);

    std::cout << "Depth: " << (depth - 1) << std::endl;
    std::cout << "Took " << duration.count() << " seconds for " << ret.states_evaluated << " states" << std::endl;
    std::cout << "Heuristic " << ret.heuristic << " with action " << ret.action << std::endl;
        
    // Clean up the history table (check 10% of entries for expiration)
    std::cout << "History table size: " << history_table.scores.size() << std::endl;
    std::cout << "Number of buckets: " << history_table.scores.bucket_count() << std::endl;
    history_table.decay(history_table.scores.size() / 10, state.turn - 20);
    std::cout << "New History table size: " << history_table.scores.size() << std::endl;

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

    // Reset resource so that it can be accessed later
    idmm_busy.clear();

    turn_end = std::chrono::steady_clock::now();
    
    // Start pondering thread
    pondering_stop = false;
    pondering_thread = std::thread([&] {
        Skaia::State state_copy = state;
        pondering_depth = 2;
        pondering_move.clear();
        std::cout << "Joining idmm_thread" << std::endl;
        idmm_thread.join(); // Wait for idmm_thread
        std::cout << "Joined idmm_thread" << std::endl;
        while (!pondering_stop)
        {
            std::cout << "p1" << std::endl;
            auto new_pondering_move = Skaia::pondering_minimax(state_copy,
                    ((state_copy.turn + 1) % 2 ? Skaia::Black : Skaia::White),
                    pondering_depth,
                    std::numeric_limits<int>::lowest(),
                    std::numeric_limits<int>::max(),
                    history_table,
                    pondering_stop);
            std::cout << "p2" << std::endl;
            while (pondering_busy.test_and_set() && !pondering_stop);
            std::cout << "p3" << std::endl;
            if (pondering_stop)
            {
                pondering_busy.clear();
                break;
            }
            std::cout << "p4" << std::endl;
            pondering_move = new_pondering_move;
            pondering_depth += 1;
            pondering_busy.clear();
            std::cout << "p5" << std::endl;
        }
    });

    return true; // to signify we are done with our turn.
}

