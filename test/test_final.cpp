#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <random>

#include "../final_magic.h"
#include "../parser.h"
#include "../simulator.h"

class FinalTest : public testing::Test {
public:
    FinalTest()
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

TEST_F(FinalTest, RunToItem)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 15 10 266", // we have running shoes!
        "POWERUP TOMATO 10 9 9"
    };
    // clang-format on
    TickDescription state = parseTickDescription(info);
    const std::map<int, float> points;
    Simulator::NewPoints newPoints;

    const auto distance2 = [](int x, int y) -> float { return static_cast<float>(std::max(x, y) - std::min(x, y)); };
    const auto distance = [&distance2](int x1, int y1, int x2, int y2) -> float { return distance2(x1, x2) + distance2(y1, y2); };

    while (!state.mPowerUps.empty()) {
        FinalMagic magic(mGameDescripton);
        magic.SetTickTimeout(std::chrono::hours { 1000 });
        magic.mGaborMagic.SetLevelLimit(2);

        mSimulator->SetState(state);
        const auto move = magic.Tick(state, points);
        mSimulator->SetVampireMove(1, move);

        const auto oldDistance = distance(state.mMe.mX, state.mMe.mY, state.mPowerUps.front().mX, state.mPowerUps.front().mY);
        std::tie(state, newPoints) = mSimulator->Tick();

        if (!state.mPowerUps.empty()) {
            const auto newDistance = distance(state.mMe.mX, state.mMe.mY, state.mPowerUps.front().mX, state.mPowerUps.front().mY);
            if (newDistance > 0) {
                if (oldDistance >= 3) {
                    ASSERT_EQ(newDistance, oldDistance - 3);
                } else if (oldDistance >= 2) {
                    ASSERT_EQ(newDistance, oldDistance - 2);
                } else {
                    ASSERT_EQ(newDistance, oldDistance - 1);
                }
            }
        }
    }
    SUCCEED();
}

TEST_F(FinalTest, HangLevel7)
{
    SetupGame(21);

    // clang-format off
    const std::vector<std::string> info = {
        "REQ 6493 0 1",
        "VAMPIRE 1 17 10 3 3 4 0",
        "VAMPIRE 4 2 19 2 1 3 0",
        "POWERUP TOMATO 19 17 10",
        "BAT1 8 17",
        "BAT2 7 1 8 1 10 1 13 1 9 18 12 19",
        "BAT3 6 1 12 1 14 1 7 18 6 19 7 19 8 19 9 19 10 19 11 19",
    };
    // clang-format on
    TickDescription state = parseTickDescription(info);

    FinalMagic magic(mGameDescripton);
    magic.SetTickTimeout(std::chrono::hours { 1000 });
    magic.mGaborMagic.SetLevelLimit(3);

    Simulator simulator(mGameDescripton);
    Simulator::NewPoints points;

    {
        const auto move = magic.Tick(state, points);
        EXPECT_FALSE(move.mSteps.empty());
        simulator.SetState(state);
        simulator.SetVampireMove(1, move);
        std::tie(state, points) = simulator.Tick();
    }

    const auto move = magic.Tick(state, points);
    EXPECT_FALSE(move.mSteps.empty());
}

/*
TEST_F(FinalTest, RandomMapHang)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 3 1 2 0",
    };
    // clang-format on
    const TickDescription originalState = parseTickDescription(info);

    std::mt19937 engine(0);
    std::uniform_int_distribution<int> coorDist(1, mGameDescripton.mMapSize - 1);
    std::uniform_int_distribution<int> batDist(1, 10);

    for (size_t testCase = 0; testCase < 10; ++testCase) {
        batDist
    }
}
*/