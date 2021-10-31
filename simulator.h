#pragma once

#include "models.h"
#include <map>
#include <set>

class Simulator {
public:
    using Area = std::set<std::pair<int, int>>;
    struct BlowArea {
        Area mArea;
        int mTickCount = -1;
        std::set<int> mVampireIds;
    };

public:
    explicit Simulator(const GameDescription& gameDescription);
    void SetState(const TickDescription& state);
    void SetVampireMove(int id, const Answer& move);
    TickDescription Tick();
    bool IsValidMove(int id, const Answer& move);
    std::vector<BlowArea> GetBlowAreas();

private:
    void RecalculateTicks(TickDescription& state);
    void RemoveDisappearedPowerups(TickDescription& state);
    void PowerupPickUp(TickDescription& state);
    void BlowUpGrenades(TickDescription& state);
    void PlantGrenades(TickDescription& state);
    void Move(TickDescription& state);
    Area GetBlowArea(const Grenade& grenade, const TickDescription& state);

private:
    GameDescription mGameDescription;
    TickDescription mState;
    std::map<int, Answer> mVampireMoves;
    std::map<std::pair<int, int>, std::pair<const Grenade*, bool>> mGrenadesByPos;
};