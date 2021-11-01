#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../mcts.h"

TEST(MCTS, Basic)
{
    GameDescription gd;
    gd.mGameId = 1;
    gd.mGrenadeRadius = 2;
    gd.mMapSize = 10;

    TickDescription td;
    td.mMe.mId = 8;
    td.mMe.mPlacableGrenades = 1;
    td.mMe.mHealth = 3;
    td.mMe.mX = 1;
    td.mMe.mY = 1;

    td.mEnemyVampires.resize(3);
    td.mEnemyVampires[0].mId = 1;
    td.mEnemyVampires[1].mId = 2;
    td.mEnemyVampires[2].mId = 3;

    td.mRequest.mTick = 1;

    MonteCarloTreeSearch mcts(td, gd, { 8, 1, 2, 3 });

    for (size_t i = 0; i < 100; ++i) {
        mcts.Step();
    }
}