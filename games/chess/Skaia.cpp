#include "Skaia.h"

namespace Skaia
{
    int file_to_skaia(const std::string& file) { return file[0] - 'a'; }
    std::string file_from_skaia(int file) { return std::string(1, 'a' + static_cast<char>(file)); }
    int rank_to_skaia(int rank) { return 8 - rank; }
    int rank_from_skaia(int rank) { return 8 - rank; }
    Type type_to_skaia(const std::string& type)
    {
        switch (type[2])
        {
            case 'w': return Pawn; break;
            case 's': return Bishop; break;
            case 'i': return Knight; break;
            case 'o': return Rook; break;
            case 'e': return Queen; break;
            case 'n': return King; break;
            default: return Empty;
        }
    }
    std::string type_from_skaia(Type type) { return names[type]; }
}

Skaia::Color operator!(const Skaia::Color &color) { return color ? Skaia::White : Skaia::Black; }

std::ostream& operator<<(std::ostream& out, const Skaia::Color& color)
{
    return out << (color == Skaia::White ? "White" : "Black");
}

std::ostream& operator<<(std::ostream& out, const Skaia::Position& pos)
{
    return out << Skaia::file_from_skaia(pos.file) << Skaia::rank_from_skaia(pos.rank);
}

