#include <algorithm>
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
    gd.mMaxTick = 100;

    TickDescription td;
    td.mMe.mId = 8;
    td.mMe.mPlacableGrenades = 1;
    td.mMe.mGrenadeRange = 2;
    td.mMe.mHealth = 3;
    td.mMe.mX = 1;
    td.mMe.mY = 1;

    td.mBat1.resize(1);
    td.mBat1[0].mX = 9;
    td.mBat1[0].mY = 9;
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
            if (!search.CalculateNextLevel(std::chrono::steady_clock::now() + std::chrono::hours(200))) {
                std::cerr << "calculate timeouted at level " << i << std::endl;
                break;
            }
        }
        const auto move = search.GetBestMove();

        std::cerr << " grenade: " << move.mPlaceGrenade << " moves: ";
        for (const auto& s : move.mSteps) {
            std::cerr << s << ", ";
        }
        std::cerr << std::endl << std::endl;

        const auto originalX = td.mMe.mX;
        const auto originalY = td.mMe.mY;

        simulator.SetState(td);
        simulator.SetVampireMove(8, move);
        td = simulator.Tick().first;

        EXPECT_EQ(td.mMe.mHealth, 3);

        // std::cerr << "position: " << td.mMe.mX << ", " << td.mMe.mY << std::endl;

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

    EXPECT_TRUE(td.mAllBats.empty());
}

class SearchTest : public testing::Test {
public:
    SearchTest()
    {
        SetupGame(11);
    }

    void SetupGame(const int mapSize)
    {
        const std::vector<std::string> startInfo = {
            "MESSAGE OK",
            "LEVEL 1",
            "GAMEID 775",
            "TEST 1",
            "MAXTICK 500",
            "GRENADERADIUS 2",
            "SIZE " + std::to_string(mapSize),
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

TEST_F(SearchTest, StuckInMap10)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 3 1 2 0",
        "BAT1 1 3",
        "BAT2 3 1"
    };
    // clang-format on
    TickDescription state = parseTickDescription(info);

    for (size_t tick = 0; tick <= 6; ++tick) {
        Search search(state, mGameDescripton, 1);
        for (size_t i = 0; i < 5; ++i) {
            search.CalculateNextLevel(std::chrono::steady_clock::now() + std::chrono::hours(100));
        }

        mSimulator->SetState(state);
        const auto move = search.GetBestMove();

        std::cerr << " grenade: " << move.mPlaceGrenade << " moves: ";
        for (const auto& s : move.mSteps) {
            std::cerr << s << ", ";
        }
        std::cerr << std::endl << std::endl;

        mSimulator->SetVampireMove(1, move);
        Simulator::NewPoints newPoints;
        std::tie(state, newPoints) = mSimulator->Tick();

        EXPECT_EQ(state.mMe.mHealth, 3);
    }
}

TEST_F(SearchTest, StuckInMap7)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 3 3 3 1 2 0",
        "BAT3 1 4",
        "BAT2 4 1 3 5 5 3"
    };
    // clang-format on
    TickDescription state = parseTickDescription(info);

    Search search(state, mGameDescripton, 1);
    for (size_t i = 0; i < 6; ++i) {
        search.CalculateNextLevel(std::chrono::steady_clock::now() + std::chrono::hours(100));
    }

    mSimulator->SetState(state);
    const auto move = search.GetBestMove();

    mSimulator->SetVampireMove(1, move);
    Simulator::NewPoints newPoints;
    std::tie(state, newPoints) = mSimulator->Tick();

    EXPECT_FALSE(state.mGrenades.empty());
}

TEST_F(SearchTest, MultipleGrenades)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 3 3 2 0",
        "BAT1 3 1 3 3 3 5",
    };
    // clang-format on
    TickDescription state = parseTickDescription(info);

    for (size_t tick = 0; tick < 3; ++tick) {
        Search search(state, mGameDescripton, 1);
        for (size_t i = 0; i < 3; ++i) {
            search.CalculateNextLevel(std::chrono::steady_clock::now() + std::chrono::hours(100));
        }

        mSimulator->SetState(state);
        const auto move = search.GetBestMove();

        mSimulator->SetVampireMove(1, move);
        Simulator::NewPoints newPoints;
        std::tie(state, newPoints) = mSimulator->Tick();
    }

    mSimulator->SetState(state);
    const auto areas = mSimulator->GetBlowAreas();
    EXPECT_EQ(areas.size(), 1);
    EXPECT_TRUE(areas[0].mArea.find(state.mBat1[0].mX, state.mBat1[0].mY));
    EXPECT_TRUE(areas[0].mArea.find(state.mBat1[1].mX, state.mBat1[1].mY));
    EXPECT_TRUE(areas[0].mArea.find(state.mBat1[2].mX, state.mBat1[2].mY));
}

TEST_F(SearchTest, MultipleGrenadesEasy)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 3 3 1 0",
        "BAT1 2 1 2 3 2 5",
    };
    // clang-format on
    TickDescription state = parseTickDescription(info);

    for (size_t tick = 0; tick < 3; ++tick) {
        Search search(state, mGameDescripton, 1);
        for (size_t i = 0; i < 3; ++i) {
            search.CalculateNextLevel(std::chrono::steady_clock::now() + std::chrono::hours(100));
        }

        mSimulator->SetState(state);
        const auto move = search.GetBestMove();

        mSimulator->SetVampireMove(1, move);
        Simulator::NewPoints newPoints;
        std::tie(state, newPoints) = mSimulator->Tick();
    }

    mSimulator->SetState(state);
    const auto areas = mSimulator->GetBlowAreas();
    EXPECT_EQ(areas.size(), 3);
    const auto it1
        = std::find_if(areas.begin(), areas.end(), [&state](const auto& element) { return element.mArea.find(state.mBat1[0].mX, state.mBat1[0].mY); });
    const auto it2
        = std::find_if(areas.begin(), areas.end(), [&state](const auto& element) { return element.mArea.find(state.mBat1[1].mX, state.mBat1[1].mY); });
    const auto it3
        = std::find_if(areas.begin(), areas.end(), [&state](const auto& element) { return element.mArea.find(state.mBat1[2].mX, state.mBat1[2].mY); });
    EXPECT_TRUE(it1 != areas.end());
    EXPECT_TRUE(it2 != areas.end());
    EXPECT_TRUE(it3 != areas.end());
    EXPECT_TRUE(it1 != it2);
    EXPECT_TRUE(it2 != it3);
}

TEST_F(SearchTest, OptimalStart)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 3 1 2 0",
        "VAMPIRE 2 9 1 3 1 2 0",
        "VAMPIRE 3 9 9 3 1 2 0",
        "VAMPIRE 4 1 9 3 1 2 0",
        "BAT1 4 1 5 1 6 1 3 2 7 2 2 3 3 3 7 3 8 3 1 4 9 4 1 5 9 5 1 6 9 6 2 7 3 7 7 7 8 7 3 8 7 8 4 9 5 9 6 9",
        "BAT2 5 2 4 3 6 3 3 4 7 4 2 5 8 5 3 6 7 6 4 7 6 7 5 8",
        "BAT3 5 3 5 4 3 5 4 5 5 5 6 5 7 5 5 6 5 7",
    };
    // clang-format on
    TickDescription state = parseTickDescription(info);

    for (size_t tick = 0; tick < 8; ++tick) {
        Search search(state, mGameDescripton, 1);
        for (size_t i = 0; i < 5; ++i) {
            search.CalculateNextLevel(std::chrono::steady_clock::now() + std::chrono::hours(100));
        }

        mSimulator->SetState(state);
        std::cerr << "move in tick " << tick + 1 << std::endl;
        const auto move = search.GetBestMove();

        if (state.mMe.mPlacableGrenades > 0 && tick > 1) {
            EXPECT_TRUE(move.mPlaceGrenade);
        }

        mSimulator->SetVampireMove(1, move);
        Simulator::NewPoints newPoints;
        std::tie(state, newPoints) = mSimulator->Tick();
    }
}

TEST_F(SearchTest, Light)
{
    SetupGame(19);

    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 547 1",
        "VAMPIRE 1 3 3 2 15 10 266"
    };
    // clang-format on
    TickDescription state = parseTickDescription(info);

    while (state.mMe.mHealth > 0) {
        Search search(state, mGameDescripton, 1);
        for (size_t i = 0; i < 2; ++i) {
            search.CalculateNextLevel(std::chrono::steady_clock::now() + std::chrono::hours(1000));
        }

        mSimulator->SetState(state);
        const auto move = search.GetBestMove();
        mSimulator->SetVampireMove(1, move);

        const auto oldHealth = state.mMe.mHealth;

        Simulator::NewPoints newPoints;
        std::tie(state, newPoints) = mSimulator->Tick();

        if (state.mMe.mHealth < oldHealth) {
            ASSERT_EQ(state.mMe.mX, 9);
            ASSERT_EQ(state.mMe.mY, 9);

            ASSERT_TRUE(mSimulator->GetLitArea().find(9, 9));
        }
    }
    SUCCEED();
}
