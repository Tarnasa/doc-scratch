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

            bool operator==(const Action& rhs) const
            {
                return from == rhs.from && to == rhs.to && promotion == rhs.promotion;
            }
            bool operator!=(const Action& rhs) const
            {
                return from != rhs.from && to != rhs.to && promotion != rhs.promotion;
            }
            bool operator<(const Action& rhs) const
            {
                return (from < rhs.from || (from == rhs.from &&
                        to < rhs.to || (to == rhs.to &&
                        promotion < rhs.promotion)));
            }
    };
}

std::ostream& operator<<(std::ostream& out, const Skaia::Action& action);

// From: http://en.cppreference.com/w/cpp/utility/hash
namespace std
{
    template<> struct hash<Skaia::Action> {
        size_t operator() (const Skaia::Action &a) const
        {
            return std::hash<int>()(a.from.rank) ^
                (std::hash<int>()(a.from.file) << 1) ^
                (std::hash<int>()(a.to.rank) << 2) ^
                (std::hash<int>()(a.to.file) << 3) ^
                (std::hash<int>()(static_cast<int>(a.promotion)) << 4);
        }
    };
}

