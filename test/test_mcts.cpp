#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../mcts.h"

TEST(MCTS, Basic)
{
    GameDescription gd;
    gd.mGameId = 1;
    gd.mGrenadeRadius = 2;
    gd.mMapSize = 5;

    TickDescription td;
    td.mMe.mId = 8;
    td.mMe.mPlacableGrenades = 1;
    td.mMe.mGrenadeRange = 2;
    td.mMe.mHealth = 3;
    td.mMe.mX = 1;
    td.mMe.mY = 1;

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

    MonteCarloTreeSearch mcts(td, gd, { 8, 1, 2, 3 });

    for (size_t i = 0; i < 100; ++i) {
        mcts.Step();
    }
    const auto move = mcts.GetBestMove();
    EXPECT_FALSE(move.mSteps.empty());
}