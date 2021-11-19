#pragma once

#include "action_sequence.h"
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

        bool operator==(const Area& other) const
        {
            return mAreas == other.mAreas && mMapSize == other.mMapSize && mVectorIsValid == other.mVectorIsValid && mVector == other.mVector;
        }
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

private:
    struct ThrowPositions {
        std::pair<int, int> origin;
        std::pair<int, int> target;
    };

    struct pair_hash {
        template <class T1, class T2> std::size_t operator()(const std::pair<T1, T2>& pair) const
        {
            return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
        }
    };

public:
    explicit Simulator(const GameDescription& gameDescription);
    void SetState(const TickDescription& state);
    void SetVampireMove(int id, const Answer& move);
    std::pair<TickDescription, NewPoints> Tick();
    bool IsValidMove(int id, const Answer& move) const;
    bool IsValidMove(int id, const ActionSequence& move, const int throwDistance = -1) const;
    std::vector<BlowArea> GetBlowAreas(const bool blowNow = false);
    Area GetBlowArea(const Grenade& grenade);
    const Area& GetReachableArea() const;
    const Area& GetThrowableArea() const;
    const Area& GetLitArea() const;
    void KillVampire(int id);

    // position is x, y
    std::optional<ThrowPositions> GetThrowPosition(const std::pair<int, int>& position, const Throw& th) const;

private:
    void RecalculateTicks();
    void RemoveDisappearedPowerups();
    void PowerupPickUp();
    void BlowUpGrenades();
    void HitLight();
    void PlantGrenades();
    void ThrowGrenades();
    void Move();

private:
    GameDescription mGameDescription;
    TickDescription mState;
    std::unordered_map<int, Answer> mVampireMoves;
    NewPoints mNewPoints;
    Area mReachableArea;
    Area mThrowableArea;
    Area mLitArea;
    bool mValid = false;
};