#pragma once

#include "../models.h"
#include "../simulator.h"

#include "levels.h"

#include <map>

class Game {
public:
    Game(const GameDescription& gd, const TickDescription& zeroTick, int playerCount);
    void SetVampireMove(int id, const Answer& move);
    std::pair<TickDescription, std::vector<std::pair<int, float>>> Tick();
    float GetPoint(int id);
    void KillVampire(int id);

private:
    void GeneratePowerups(TickDescription& tick);

private:
    Level mLevel;
    GameDescription mGameDescription;
    Simulator mSimulator;
    std::map<int, float> mCumulatedPoints;
    int mNextPowerupTick = 0;
    TickDescription mPrevTick;
};