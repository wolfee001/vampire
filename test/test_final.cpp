#include <gmock/gmock.h>
#include <gtest/gtest.h>

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

TEST_F(FinalTest, DISABLED_RunToItem)
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

        mSimulator->SetState(state);
        const auto move = magic.Tick(state, points);
        mSimulator->SetVampireMove(1, move);

        const auto oldDistance = distance(state.mMe.mX, state.mMe.mY, state.mPowerUps.front().mX, state.mPowerUps.front().mY);
        std::tie(state, newPoints) = mSimulator->Tick();

        if (!state.mPowerUps.empty()) {
            const auto newDistance = distance(state.mMe.mX, state.mMe.mY, state.mPowerUps.front().mX, state.mPowerUps.front().mY);
            ASSERT_EQ(newDistance, oldDistance - 3);
        }
    }
    SUCCEED();
}
