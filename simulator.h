#pragma once

#include "models.h"
#include <bitset>
#include <map>
#include <set>

class Simulator {
public:
    struct Area {
        using AreaVector = std::vector<std::pair<int, int>>;
        std::bitset<23 * 23> mAreas;
        size_t mMapSize;
        bool mVectorIsValid = false;
        AreaVector mVector;

        Area(int mapSize)
            : mMapSize(static_cast<size_t>(mapSize))
        {
        }
        bool find(int x, int y) const;
        void insert(int x, int y);
        const AreaVector& getAsVector() const;
    };
    struct BlowArea {
        BlowArea(int mapSize)
            : mArea(mapSize)
        {
        }

        Area mArea;
        int mTickCount = -1;
        std::set<int> mVampireIds;
    };

    using NewPoints = std::map<int, float>; // indexed by player id

public:
    explicit Simulator(const GameDescription& gameDescription);
    void SetState(const TickDescription& state);
    void SetVampireMove(int id, const Answer& move);
    std::pair<TickDescription, NewPoints> Tick();
    bool IsValidMove(int id, const Answer& move) const;
    std::vector<BlowArea> GetBlowAreas(const bool blowNow = false);
    Area GetBlowArea(const Grenade& grenade, const TickDescription& state);
    std::map<int, Answer> GetMoves(const TickDescription& newState);
    const Area& GetReachableArea() const;
    const Area& GetLitArea() const;

private:
    void RecalculateTicks(TickDescription& state);
    void RemoveDisappearedPowerups(TickDescription& state);
    void PowerupPickUp(TickDescription& state);
    void BlowUpGrenades(TickDescription& state);
    void HitLight(TickDescription& state);
    void PlantGrenades(TickDescription& state);
    void Move(TickDescription& state);

private:
    GameDescription mGameDescription;
    TickDescription mState;
    std::map<int, Answer> mVampireMoves;
    NewPoints mNewPoints;
    Area mReachableArea;
    Area mLitArea;
};