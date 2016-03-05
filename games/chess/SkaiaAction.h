#pragma once

#include <bitset>
#include <vector>
#include <string>

#include "Skaia.h"

namespace Skaia
{
    class Action
    {
        public:
            Position from, to;
            Type promotion;

            Action(const Position& from, const Position& to, Type promotion) :
                from(from), to(to), promotion(promotion) {}

            bool operator!=(const Action& rhs)
            {
                return from != rhs.from && to != rhs.to && promotion != rhs.promotion;
            }
    };

    /*
    class SmallAction
    {
        public:
            // 3 bits for each of from rank/file and to rank/file, last 4 bits indicate posible promotion
            std::bitset<16> data;

            // The important stuff
            SmallAction(size_t from_rank, size_t from_file, size_t to_rank, size_t to_file, size_t promotion=4) :
                data(from_rank | from_file << 3 | to_rank << 6 | to_file << 9 | promotion << 12) {}

            size_t get_from_rank() const { return data & 0b111; }
            size_t get_from_file() const { return (data >> 3) & 0b111; }
            size_t get_to_rank() const { return (data >> 6) & 0b111; }
            size_t get_to_file() const { return (data >> 9) & 0b111; }
            size_t get_promotion() const { return (data >> 12) & 0b1111; }

            Action to_action() const
            {
                return Action(get_from_rank(), get_from_file(), get_to_rank(), get_to_file(), get_promotion());
            }

            // The needed but not too important stuff
            SmallAction(const SmallAction& action) = default;
            SmallAction& operator=(const SmallAction& action) = default;
    };
    */
}

