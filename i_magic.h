#pragma once

#include "models.h"
#include "simulator.h"
#include <chrono>

class IMagic {
public:
    explicit IMagic(const GameDescription& gameDescription)
        : mGameDescription(gameDescription)
    {
    }

    virtual ~IMagic() = default;

    virtual Answer Tick(const TickDescription& tickDescription, const Simulator::NewPoints& points) = 0;
    void SetTickTimeout(std::chrono::milliseconds millis);

protected:
    GameDescription mGameDescription;
    std::chrono::milliseconds mTimeout = std::chrono::milliseconds(1000);
};