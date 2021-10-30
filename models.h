#pragma once

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
    int mPoint = -1;
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

struct Answer {
    bool mPlaceGrenade = false;
    std::vector<char> mSteps;
};