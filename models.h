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
};

struct Request {
    int mGameId = -1;
    int mTick = -1;
    int mVampireId = -1;
};

struct Vampire {
    int mId = -1;
    int mX = -1;
    int mY = -1;
    int mHealth = -1;
    int mPlacableGrenades = -1;
    int mGrenadeRange = -1;
    int mRunningShoesTick = 0;
};

struct Grenade {
    int mId = -1;
    int mX = -1;
    int mY = -1;
    int mTick = -1;
    int mRange = -1;
};

struct PowerUp {
    enum class Type { Tomato, Grenade, Battery, Shoe };

    Type mType = Type::Tomato;
    int mRemainingTick = 0;
    int mX = -1;
    int mY = -1;
};

struct BatSquad {
    int mDensity = -1;
    int mX = -1;
    int mY = -1;
};

struct EndMessage {
    int mPoint = -1;
    std::string mReason;
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
};
