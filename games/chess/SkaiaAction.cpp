#include "SkaiaAction.h"

std::ostream& operator<<(std::ostream& out, const Skaia::Action& action)
{
    return out << "Act(" << action.from << " to " << action.to << " (" << Skaia::type_from_skaia(action.promotion) << ")";
}

