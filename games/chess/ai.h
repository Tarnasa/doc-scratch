// This is where you build your AI for the Checkers game.

#ifndef JOUEUR_CHESS_AI_H
#define JOUEUR_CHESS_AI_H

#include "chess.h"
#include "game.h"
#include "gameObject.h"
#include "move.h"
#include "piece.h"
#include "player.h"

#include "../../joueur/baseAI.h"

#include <chrono>
#include <map>
#include <cstdint>
#include <atomic>
#include <thread>
#include <unordered_map>

#include "SkaiaState.h"
#include "SkaiaMM.h"

/// <summary>
/// This the header file for where you build your AI for the Chess game.
/// </summary>
class Chess::AI : public Joueur::BaseAI
{
    public:
        /// Custom stuffs
        Skaia::State state; // Store the board state between turns
        std::chrono::time_point<std::chrono::steady_clock> turn_end; // Used to measure how long the opponent is taking
        std::map<int, std::chrono::nanoseconds> average_time; // Holds the average time (in ns) it has taken to traverse down to the given depth

        std::thread idmm_thread;
        std::atomic<bool> idmm_stop;
        std::atomic_flag idmm_busy; // Set when a resource is busy

        std::thread pondering_thread;
        std::atomic<bool> pondering_stop;
        std::atomic_flag pondering_busy;
        std::vector<std::pair<Skaia::Action, Skaia::MMReturn>> pondering_move;
        int pondering_depth;

        std::unordered_map<Skaia::Action, std::pair<int, int>> history_table;


        /// <summary>
        /// This is a pointer to the Game object itself, it contains all the information about the current game
        /// </summary>
        Chess::Game* game;

        /// <summary>
        /// This is a pointer to your AI's player. This AI class is not a player, but it should command this Player.
        /// </summary>
        Chess::Player* player;


        /// <summary>
        /// This returns your AI's name to the game server. Just replace the string.
        /// </summary>
        /// <returns>string of you AI's name.</returns>
        std::string getName();

        /// <summary>
        /// This is automatically called when the game first starts, once the Game object and all GameObjects have been initialized, but before any players do anything.
        /// </summary>
        void start();

        /// <summary>
        /// This is automatically called every time the game (or anything in it) updates.
        /// </summary>
        void gameUpdated();

        /// <summary>
        /// This is automatically called when the game ends.
        /// </summary>
        /// <param name="won">true if your player won, false otherwise</param>
        /// <param name="reason">a string explaining why you won or lost</param>
        void ended(bool won, std::string reason);


        /// <summary>
        /// This is called every time it is this AI.player's turn.
        /// </summary>
        /// <returns>Represents if you want to end your turn. True means end your turn, False means to keep your turn going and re-call this function.</returns>
        bool runTurn();
};

#endif
