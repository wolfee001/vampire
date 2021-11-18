#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

struct GameDescription {
    int mLevelId = -1;
    bool mIsTest = false;
    int mGameId = -1;
    int mMaxTick = -1;
    int mGrenadeRadius = 0;
    int mMapSize = 0;

    bool operator==(const GameDescription& other) const
    {
        return mLevelId == other.mLevelId && mIsTest == other.mIsTest && mGameId == other.mGameId && mMaxTick == other.mMaxTick
            && mGrenadeRadius == other.mGrenadeRadius && mMapSize == other.mMapSize;
    }
};

struct Request {
    int mGameId = -1;
    int mTick = -1;
    int mVampireId = -1;

    bool operator==(const Request& other) const
    {
        return mGameId == other.mGameId && mTick == other.mTick && mVampireId == other.mVampireId;
    }
};

struct Vampire {
    int mId = -1;
    int mX = -1;
    int mY = -1;
    int mHealth = -1;
    int mPlacableGrenades = -1;
    int mGrenadeRange = -1;
    int mRunningShoesTick = 0;

    int mGhostModeTick = 0;

    bool operator==(const Vampire& other) const
    {
        return mId == other.mId && mX == other.mX && mY == other.mY && mHealth == other.mHealth && mPlacableGrenades == other.mPlacableGrenades
            && mGrenadeRange == other.mGrenadeRange && mRunningShoesTick == other.mRunningShoesTick;
    }
};

struct Grenade {
    int mId = -1;
    int mX = -1;
    int mY = -1;
    int mTick = -1;
    int mRange = -1;

    bool operator==(const Grenade& other) const
    {
        return mId == other.mId && mX == other.mX && mY == other.mY && mTick == other.mTick && mRange == other.mRange;
    }
};

struct PowerUp {
    enum class Type { Tomato, Grenade, Battery, Shoe };

    Type mType = Type::Tomato;
    int mRemainingTick = 0;
    int mX = -1;
    int mY = -1;
    int mDefensTime = 0;

    int mKeepAliveHint = -1;

    bool operator==(const PowerUp& other) const
    {
        return mType == other.mType && mRemainingTick == other.mRemainingTick && mX == other.mX && mY == other.mY;
    }
};

struct BatSquad {
    int mDensity = -1;
    int mX = -1;
    int mY = -1;

    bool operator==(const BatSquad& other) const
    {
        return mDensity == other.mDensity && mX == other.mX && mY == other.mY;
    }
};

struct EndMessage {
    float mPoint = 0.f;
    std::string mReason;

    bool operator==(const EndMessage& other) const
    {
        return mPoint == other.mPoint && mReason == other.mReason;
    }
};

struct TickDescription {
    Request mRequest;
    std::vector<std::string> mWarnings;
    Vampire mMe;
    std::vector<Vampire> mEnemyVampires;
    std::vector<Grenade> mGrenades;
    std::vector<PowerUp> mPowerUps;
    std::vector<BatSquad> mBat1;
    std::vector<BatSquad> mBat2;
    std::vector<BatSquad> mBat3;
    std::vector<BatSquad> mAllBats;
    EndMessage mEndMessage;

    bool operator==(const TickDescription& other) const
    {
        return mRequest == other.mRequest && mWarnings == other.mWarnings && mMe == other.mMe && mEnemyVampires == other.mEnemyVampires
            && mGrenades == other.mGrenades && mPowerUps == other.mPowerUps && mBat1 == other.mBat1 && mBat2 == other.mBat2 && mBat3 == other.mBat3
            && mAllBats == other.mAllBats && mEndMessage == other.mEndMessage;
    }
};

struct Throw {
    enum class Direction { Up, Down, Left, Right, XUp, XDown, XLeft, XRight };
    Direction mDirection;
    int mDistance = -1;

    bool operator==(const Throw& other) const
    {
        return mDirection == other.mDirection && mDistance == other.mDistance;
    }
};

struct Answer {
    bool mPlaceGrenade = false;
    std::vector<char> mSteps;
    std::optional<Throw> mThrow = std::nullopt;

    bool operator==(const Answer& other) const
    {
        return mPlaceGrenade == other.mPlaceGrenade && mSteps == other.mSteps && mThrow == other.mThrow;
    }
};

struct pos_t {
    int y;
    int x;
    pos_t(int _y = -1, int _x = -1)
        : y(_y)
        , x(_x)
    {
    }
    pos_t GetPos(int d) const
    {
        pos_t dir[5] = { { -1, 0 }, { 0, 1 }, { 1, 0 }, { 0, -1 }, { 0, 0 } };
        return pos_t(y + dir[d].y, x + dir[d].x);
    }

    int GetDir(pos_t p2) const
    {
        if (p2.y < y) {
            return 0;
        }
        if (p2.x > x) {
            return 1;
        }
        if (p2.y > y) {
            return 2;
        }
        if (p2.x < x) {
            return 3;
        }
        return -1;
    }
    int GetDist(const pos_t& o) const // rough distance
    {
        int dx = abs(x - o.x);
        int dy = abs(y - o.y);
        if (dx == 0 && dy == 0)
            return 0;
        if (dx == 0 && x % 2 == 0)
            dx += 2;
        if (dy == 0 && y % 2 == 0)
            dy += 2;
        return dx + dy;
    }
    bool operator==(const pos_t& o) const
    {
        return x == o.x && y == o.y;
    }
    bool operator<(const pos_t& o) const
    {
        return x < o.x || (x == o.x && y < o.y);
    }
    bool operator!=(const pos_t& o) const
    {
        return x != o.x || y != o.y;
    }
    bool isvalid() const
    {
        return x != -1;
    }
    friend std::ostream& operator<<(std::ostream& stream, const pos_t& p)
    {
        return stream << p.y << "(y):" << p.x << "(x) ";
    }
};

enum phase_t { NONE, PHASE1, ITEM, BETWEEN_ITEMS, CHARGE };
