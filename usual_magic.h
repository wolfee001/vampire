// clang-format off
#pragma once

#include "models.h"

class UsualMagic {
public:
    struct Answer {
        bool mPlaceGrenade = false;
        std::vector<std::string> mSteps;
    };

public:
    explicit UsualMagic(const GameDescription& gameDescription);

    Answer Tick(const TickDescription& tickDescription);

private:
    GameDescription mGameDescription;
};
// clang-format on