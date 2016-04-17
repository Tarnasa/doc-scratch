/// Stores a "score" and "age" for Skaia::Action's so that
///  pruning can be done better by ordering actions by their
///  score in this table.

#pragma once

#include "SkaiaAction.h"

#include <unordered_map>

class HistoryTable
{
    public:
        // First of pair is score
        // Second of pair is the turn it was updated on
        std::unordered_map<Skaia::Action, std::pair<int, int>> scores;

        HistoryTable() = default;
        HistoryTable(const HistoryTable &other) = default;
        HistoryTable& operator=(const HistoryTable &other) = default;

        // Increases an action's score and sets the turn it was updated on
        void increase(const Skaia::Action &action, int amount, int turn);

        // Returns the score for the given action, zero if not found
        int get_score(const Skaia::Action &action);

        // Looks at `number_to_check` entries and if their turns are earlier
        //  than `threshold` then they are removed from the table.
        void decay(int number_to_check, int threshold);
};

std::ostream& operator<<(std::ostream& out, const HistoryTable& ht);

