#include "HistoryTable.h"

#include <random>

void HistoryTable::increase(const Skaia::Action &action, int amount, int turn)
{
    if (scores.count(action) > 0)
    {
        scores[action].first += amount;
        scores[action].second = turn;
    }
    else
    {
        scores.emplace(action, std::make_pair(amount, turn));
    }
}

int HistoryTable::get_score(const Skaia::Action &action)
{
    if (scores.count(action) > 0)
    {
        return scores[action].first;
    }
    else
    {
        return 0;
    }
}

void HistoryTable::decay(int number_to_check, int threshold)
{
    static auto rand = std::default_random_engine(0);

    for (int i = 0; i < number_to_check; ++i)
    {
        // Stop if their aint none left
        if (scores.size() == 0) break;
        // Choose a random bucket
        int bucket, bucket_size;
        do
        { 
            bucket = std::uniform_int_distribution<int>(0, scores.bucket_count())(rand);
        }
        while ( (bucket_size = scores.bucket_size(bucket)) == 0 );

        // Select a random element from the bucket
        const auto &key_value = std::next(scores.cbegin(bucket),
                std::uniform_int_distribution<int>(0, bucket_size)(rand));
        if (key_value->second.second < threshold)
        {
            // Remove the element
            scores.erase(key_value->first);
        }
    }
}

std::ostream& operator<<(std::ostream& out, const HistoryTable& ht)
{
    for (auto& key_value : ht.scores)
    {
        out << key_value.first << ": " << key_value.second.first << ", " << key_value.second.second << std::endl;
    }
    return out;
}

