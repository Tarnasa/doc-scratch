#include "SkaiaPiece.h"

std::ostream& operator<<(std::ostream& out, const Skaia::Piece& piece)
{
    out << "Piece(" << piece.color << " " << type_from_skaia(piece.type) << " at " << piece.pos << " with ";
    out << "id: " << piece.id << " alive: " << piece.alive << " special: " << piece.special << " moves:";
    for (auto& action : piece.moves)
    {
        out << " " << action;
    }
    out << ")";
    return out;
}
