#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../parser.h"

TEST(Parsing, ParseGameStart)
{
    std::vector<std::string> startInfo = {
        "MESSAGE OK",
        "LEVEL 1",
        "GAMEID 775",
        "TEST 1",
        "MAXTICK 500",
        "GRENADERADIUS 2",
        "SIZE 11",
    };

    const GameDescription description = parseGameDescription(startInfo);

    EXPECT_EQ(description.mLevelId, 1);
    EXPECT_EQ(description.mGameId, 775);
    EXPECT_EQ(description.mIsTest, true);
    EXPECT_EQ(description.mMaxTick, 500);
    EXPECT_EQ(description.mGrenadeRadius, 2);
    EXPECT_EQ(description.mMapSize, 11);
}

TEST(Parsing, ParseTick)
{
    std::vector<std::string> info;
    TickDescription desc;
    // clang-format off
    info = { 
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 3 1 2 0",
        "VAMPIRE 3 9 9 3 1 2 0",
        "VAMPIRE 4 1 9 3 1 2 0",
        "VAMPIRE 2 9 1 3 1 2 0",
        "BAT1 4 1 5 1 6 1 3 2 7 2 2 3 3 3 7 3 8 3 1 4 9 4 1 5 9 5 1 6 9 6 2 7 3 7 7 7 8 7 3 8 7 8 4 9 5 9 6 9",
        "BAT2 5 2 4 3 6 3 3 4 7 4 2 5 8 5 3 6 7 6 4 7 6 7 5 8",
        "BAT3 5 3 5 4 3 5 4 5 5 5 6 5 7 5 5 6 5 7",
        "GRENADE 1 1 3 15 2",
        "GRENADE 1 9 3 0 2",
        "POWERUP TOMATO -3 2 1",
        "POWERUP GRENADE 2 3 1",
        "POWERUP BATTERY 10 7 1",
        "POWERUP SHOE -10 8 1"
    };
    // clang-format on
    desc = parseTickDescription(info);

    // clang-format off
    info = { 
        "REQ 775 0 1"
    };
    // clang-format on
    desc = parseTickDescription(info);
    EXPECT_EQ(desc.mRequest.mGameId, 775);
    EXPECT_EQ(desc.mRequest.mTick, 0);
    EXPECT_EQ(desc.mRequest.mVampireId, 1);

    // clang-format off
    info = { 
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 3 1 2 0"
    };
    // clang-format on
    desc = parseTickDescription(info);
    EXPECT_EQ(desc.mEnemyVampires.size(), 0);
    EXPECT_EQ(desc.mMe.mId, 1);
    EXPECT_EQ(desc.mMe.mX, 1);
    EXPECT_EQ(desc.mMe.mY, 1);
    EXPECT_EQ(desc.mMe.mHealth, 3);
    EXPECT_EQ(desc.mMe.mPlacableGrenades, 1);
    EXPECT_EQ(desc.mMe.mGrenadeRange, 2);
    EXPECT_EQ(desc.mMe.mRunningShoesTick, 0);

    // clang-format off
    info = { 
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 3 1 2 0",
        "VAMPIRE 3 9 9 3 1 2 0",
    };
    // clang-format on
    desc = parseTickDescription(info);
    EXPECT_EQ(desc.mEnemyVampires.size(), 1);
    EXPECT_EQ(desc.mMe.mId, 1);
    EXPECT_EQ(desc.mEnemyVampires[0].mId, 3);
    EXPECT_EQ(desc.mEnemyVampires[0].mX, 9);
    EXPECT_EQ(desc.mEnemyVampires[0].mY, 9);
    EXPECT_EQ(desc.mEnemyVampires[0].mHealth, 3);
    EXPECT_EQ(desc.mEnemyVampires[0].mPlacableGrenades, 1);
    EXPECT_EQ(desc.mEnemyVampires[0].mGrenadeRange, 2);
    EXPECT_EQ(desc.mEnemyVampires[0].mRunningShoesTick, 0);

    // clang-format off
    info = { 
        "BAT1 4 1 5 1",
        "BAT2 5 2 4 3",
        "BAT3 5 3 5 4",
    };
    // clang-format on
    desc = parseTickDescription(info);
    EXPECT_EQ(desc.mBat1.size(), 2);
    EXPECT_EQ(desc.mBat2.size(), 2);
    EXPECT_EQ(desc.mBat3.size(), 2);
    EXPECT_EQ(desc.mAllBats.size(), 6);
    EXPECT_EQ(desc.mBat1[0].mX, 4);
    EXPECT_EQ(desc.mBat1[0].mY, 1);
    EXPECT_EQ(desc.mBat1[1].mX, 5);
    EXPECT_EQ(desc.mBat1[1].mY, 1);

    // clang-format off
    info = { 
        "GRENADE 1 1 3 15 2",
        "GRENADE 1 9 3 0 2",
    };
    // clang-format on
    desc = parseTickDescription(info);
    EXPECT_EQ(desc.mGrenades.size(), 2);
    EXPECT_EQ(desc.mGrenades[0].mId, 1);
    EXPECT_EQ(desc.mGrenades[0].mX, 1);
    EXPECT_EQ(desc.mGrenades[0].mY, 3);
    EXPECT_EQ(desc.mGrenades[0].mTick, 15);
    EXPECT_EQ(desc.mGrenades[0].mRange, 2);

    // clang-format off
    info = { 
        "POWERUP TOMATO -3 2 1",
        "POWERUP GRENADE 2 3 1",
        "POWERUP BATTERY 10 7 1",
        "POWERUP SHOE -10 8 1"
    };
    // clang-format on
    desc = parseTickDescription(info);
    EXPECT_EQ(desc.mPowerUps.size(), 4);
    EXPECT_EQ(desc.mPowerUps[0].mType, PowerUp::Type::Tomato);
    EXPECT_EQ(desc.mPowerUps[1].mType, PowerUp::Type::Grenade);
    EXPECT_EQ(desc.mPowerUps[2].mType, PowerUp::Type::Battery);
    EXPECT_EQ(desc.mPowerUps[3].mType, PowerUp::Type::Shoe);
    EXPECT_EQ(desc.mPowerUps[0].mRemainingTick, -3);
    EXPECT_EQ(desc.mPowerUps[0].mX, 2);
    EXPECT_EQ(desc.mPowerUps[0].mY, 1);
}