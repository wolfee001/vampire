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

    MonteCarloTreeSearch mcts(td, gd, { 8, 1, 2, 3 });
    mcts.Step();
}