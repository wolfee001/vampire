#pragma once

#include "models.h"

#include <map>

class IMagic {
public:
    explicit IMagic(const GameDescription& gameDescription)
        : mGameDescription(gameDescription)
    {
    }

    virtual ~IMagic() = default;

    virtual Answer Tick(const TickDescription& tickDescription, const std::map<int, float>& points) = 0;

protected:
    GameDescription mGameDescription;
};