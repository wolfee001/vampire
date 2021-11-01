#pragma once

#include "models.h"

class IMagic {
public:
    explicit IMagic(const GameDescription& gameDescription)
        : mGameDescription(gameDescription)
    {
    }

    virtual ~IMagic() = default;

    virtual Answer Tick(const TickDescription& tickDescription) = 0;

protected:
    GameDescription mGameDescription;
};