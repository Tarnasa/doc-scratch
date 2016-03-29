#include "SkaiaSimpleSmallState.h"

std::ostream& operator<<(std::ostream& out, const Skaia::SimpleSmallState &sss)
{
    out << std::hex << std::setw(sizeof(uint64_t) * 2) << std::setfill('0');
    bool space = false;
    for (auto &d : sss.data)
    {
        if (space) out << " ";
        out << d;
        space = true;
    }
    out << std::dec << std::setw(0) << std::setfill(' ');
    return out;
}
