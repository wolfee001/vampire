#pragma once

#include "models.h"
#include <bitset>
#include <unordered_map>
#include <unordered_set>

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
        void clear(int x, int y);
        const AreaVector& getAsVector() const;
    };
    struct BlowArea {
        BlowArea(int mapSize)
            : mArea(mapSize)
        {
        }

        Area mArea;
        int mTickCount = -1;
        std::unordered_set<int> mVampireIds;
    };

    using NewPoints = std::unordered_map<int, float>; // indexed by player id

public:
    explicit Simulator(const GameDescription& gameDescription);
    void SetState(const TickDescription& state);
    void SetVampireMove(int id, const Answer& move);
    std::pair<TickDescription, NewPoints> Tick();
    bool IsValidMove(int id, const Answer& move) const;
    std::vector<BlowArea> GetBlowAreas(const bool blowNow = false);
    Area GetBlowArea(const Grenade& grenade);
    const Area& GetReachableArea() const;
    const Area& GetLitArea() const;
    void KillVampire(int id);

private:
    void RecalculateTicks();
    void RemoveDisappearedPowerups();
    void PowerupPickUp();
    void BlowUpGrenades();
    void HitLight();
    void PlantGrenades();
    void Move();

private:
    GameDescription mGameDescription;
    TickDescription mState;
    std::unordered_map<int, Answer> mVampireMoves;
    NewPoints mNewPoints;
    Area mReachableArea;
    Area mLitArea;
    bool mValid = false;
};