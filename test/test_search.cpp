#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../search.h"

TEST(Search, Basic)
{
    GameDescription gd;
    gd.mGameId = 1;
    gd.mGrenadeRadius = 2;
    gd.mMapSize = 11;

    TickDescription td;
    td.mMe.mId = 8;
    td.mMe.mPlacableGrenades = 1;
    td.mMe.mGrenadeRange = 2;
    td.mMe.mHealth = 3;
    td.mMe.mX = 1;
    td.mMe.mY = 1;

    td.mBat1.resize(1);
    td.mBat1[0].mX = 10;
    td.mBat1[0].mY = 10;
    td.mBat1[0].mDensity = 1;

    td.mAllBats = td.mBat1;

    td.mEnemyVampires.resize(3);
    td.mEnemyVampires[0].mId = 1;
    td.mEnemyVampires[0].mHealth = 3;
    td.mEnemyVampires[0].mX = gd.mMapSize - 2;
    td.mEnemyVampires[0].mY = 1;
    td.mEnemyVampires[0].mPlacableGrenades = 1;
    td.mEnemyVampires[0].mGrenadeRange = 2;

    td.mEnemyVampires[1].mId = 2;
    td.mEnemyVampires[1].mHealth = 3;
    td.mEnemyVampires[1].mX = gd.mMapSize - 2;
    td.mEnemyVampires[1].mY = gd.mMapSize - 2;
    td.mEnemyVampires[1].mPlacableGrenades = 1;
    td.mEnemyVampires[1].mGrenadeRange = 2;

    td.mEnemyVampires[2].mId = 3;
    td.mEnemyVampires[2].mHealth = 3;
    td.mEnemyVampires[2].mX = 1;
    td.mEnemyVampires[2].mY = gd.mMapSize - 2;
    td.mEnemyVampires[2].mPlacableGrenades = 1;
    td.mEnemyVampires[2].mGrenadeRange = 2;

    td.mRequest.mTick = 1;

    Simulator simulator(gd);

    for (size_t steps = 0; steps <= 15; ++steps) {
        Search search(td, gd, 8);
        for (size_t i = 0; i < 3; ++i) {
            search.CalculateNextLevel();
        }
        const auto move = search.GetBestMove();

        std::cerr << " grenade: " << move.mPlaceGrenade << " moves: ";
        for (const auto& s : move.mSteps) {
            std::cerr << s << ", ";
        }
        std::cerr << std::endl;

        const auto originalX = td.mMe.mX;
        const auto originalY = td.mMe.mY;

        simulator.SetState(td);
        simulator.SetVampireMove(8, move);
        td = simulator.Tick().first;

        EXPECT_EQ(td.mMe.mHealth, 3);

        std::cerr << "position: " << td.mMe.mX << ", " << td.mMe.mY << std::endl;

        int newX = originalX;
        int newY = originalY;
        for (const auto& step : move.mSteps) {
            switch (step) {
            case 'U':
                --newY;
                break;
            case 'D':
                ++newY;
                break;
            case 'L':
                --newX;
                break;
            case 'R':
                ++newX;
                break;
            }
        }
        EXPECT_EQ(newX, td.mMe.mX);
        EXPECT_EQ(newY, td.mMe.mY);

        /*
                std::cerr << search.mLevels.size() << std::endl;
                for (const auto& l : search.mLevels) {
                    std::cerr << l.size() << std::endl;
                }
                */
    }

    // EXPECT_TRUE(move.mPlaceGrenade);
}