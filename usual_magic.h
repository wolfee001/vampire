// clang-format off
#pragma once

#include "models.h"

class UsualMagic {
public:
    explicit UsualMagic(const GameDescription& gameDescription);

    Answer Tick(const TickDescription& tickDescription);

private:
    GameDescription mGameDescription;
};
// clang-format on