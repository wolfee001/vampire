#include "gabor_magic.h"

GaborMagic::GaborMagic(const GameDescription& gameDescription)
    : IMagic(gameDescription)
{
    // Maybe some constructor magic? :)
}

Answer GaborMagic::Tick(const TickDescription& /* tickDescription */)
{
    Answer answer;
    answer.mSteps = { 'R', 'R' };

    return answer;
}