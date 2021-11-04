#include "final_magic.h"
#include "models.h"

FinalMagic::FinalMagic(const GameDescription& gameDescription)
    : IMagic(gameDescription)
{
    // Maybe some constructor magic? :)
}

Answer FinalMagic::Tick(const TickDescription& tickDescription, const std::map<int, float>& points)
{
    return Answer();
}
