// clang-format off
#include "usual_magic.h"

UsualMagic::UsualMagic(const GameDescription& gameDescription)
    : mGameDescription(gameDescription)
{
    // Maybe some constructor magic? :)
    std::srand(static_cast<unsigned int>(time(nullptr)));
}

Answer UsualMagic::Tick(const TickDescription& tickDescription)
{
    Answer answer;

    std::vector<char> dirs = { 'U', 'R', 'D', 'L' };
    answer.mSteps.push_back(dirs[static_cast<size_t>(std::rand() % 4)]);
    answer.mSteps.push_back(dirs[static_cast<size_t>(std::rand() % 4)]);

    return answer;
}

// clang-format on