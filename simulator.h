#pragma once

#include "models.h"
#include <map>

class Simulator {
public:
    explicit Simulator(const GameDescription& gameDescription);
    void SetState(const TickDescription& state);
    void SetVampireMove(int id, const Answer& move);
    TickDescription Tick();

private:
    void RecalculateTicks(TickDescription& state);
    void RemoveDisappearedPowerups(TickDescription& state);
    void PowerupPickUp(TickDescription& state);
    void BlowUpGrenades(TickDescription& state);
    void PlantGrenades(TickDescription& state);

private:
    GameDescription mGameDescription;
    TickDescription mState;
    std::map<int, Answer> mVampireMoves;
};