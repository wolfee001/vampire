#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../parser.h"
#include "../search.h"
#include "../simulator.h"

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
        for (size_t i = 0; i < 10; ++i) {
            search.CalculateNextLevel(std::chrono::steady_clock::now() + std::chrono::milliseconds(100));
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

        std::cerr << search.mLevels.size() << std::endl;
        for (const auto& l : search.mLevels) {
            std::cerr << l.size() << std::endl;
        }
    }

    // EXPECT_TRUE(move.mPlaceGrenade);
}

class SearchTest : public testing::Test {
public:
    SearchTest()
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
        mGameDescripton = parseGameDescription(startInfo);

        mSimulator = std::make_unique<Simulator>(mGameDescripton);
    }

protected:
    GameDescription mGameDescripton;
    std::unique_ptr<Simulator> mSimulator;
};

TEST_F(SearchTest, BombBesideMe)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 2 1 3 1 2 0",
        "GRENADE 1 1 1 2 2",
    };
    // clang-format on
    TickDescription state = parseTickDescription(info);

    for (size_t tick = 0; tick <= 1; ++tick) {
        Search search(state, mGameDescripton, 1);
        for (size_t i = 0; i < 3; ++i) {
            search.CalculateNextLevel(std::chrono::steady_clock::now() + std::chrono::hours(100));
        }

        mSimulator->SetState(state);
        const auto move = search.GetBestMove();
        mSimulator->SetVampireMove(1, move);
        Simulator::NewPoints newPoints;
        std::tie(state, newPoints) = mSimulator->Tick();

        EXPECT_EQ(state.mMe.mHealth, 3);
    }
}

TEST_F(SearchTest, BombUnderMe)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 2 1 3 1 2 0",
        "GRENADE 1 1 1 2 2",
    };
    // clang-format on
    TickDescription state = parseTickDescription(info);

    for (size_t tick = 0; tick <= 1; ++tick) {
        Search search(state, mGameDescripton, 1);
        for (size_t i = 0; i < 3; ++i) {
            search.CalculateNextLevel(std::chrono::steady_clock::now() + std::chrono::hours(100));
        }

        mSimulator->SetState(state);
        const auto move = search.GetBestMove();
        mSimulator->SetVampireMove(1, move);
        Simulator::NewPoints newPoints;
        std::tie(state, newPoints) = mSimulator->Tick();

        EXPECT_EQ(state.mMe.mHealth, 3);
    }
}

/*
TEST_F(SearchTest, BatBesideMe)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 3 1 2 0",
        "BAT1 1 2"
    };
    // clang-format on
    TickDescription state = parseTickDescription(info);
    Simulator::NewPoints cumulativePoints;
    cumulativePoints[1] = 0;

    for (size_t tick = 0; tick <= 6; ++tick) {
        Search search(state, mGameDescripton, 1);
        for (size_t i = 0; i < 6; ++i) {
            search.CalculateNextLevel(std::chrono::steady_clock::now() + std::chrono::hours(100));
        }

        mSimulator->SetState(state);
        const auto move = search.GetBestMove();
        mSimulator->SetVampireMove(1, move);
        Simulator::NewPoints newPoints;
        std::tie(state, newPoints) = mSimulator->Tick();

        cumulativePoints.at(1) += newPoints.at(1);

        EXPECT_EQ(state.mMe.mHealth, 3);
    }
    EXPECT_EQ(cumulativePoints.at(1), 12);
}
*/