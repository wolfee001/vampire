#pragma once

#include "models.h"
#include <chrono>
#include <map>

class IMagic {
public:
    explicit IMagic(const GameDescription& gameDescription)
        : mGameDescription(gameDescription)
    {
    }

    virtual ~IMagic() = default;

    virtual Answer Tick(const TickDescription& tickDescription, const std::map<int, float>& points) = 0;
    void SetTickTimeout(std::chrono::milliseconds millis);

protected:
    GameDescription mGameDescription;
    std::chrono::milliseconds mTimeout = std::chrono::milliseconds(1000);
};