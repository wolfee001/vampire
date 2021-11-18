#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "../parser.h"
#include "../simulator.h"

class SimulateTest : public testing::Test {
public:
    SimulateTest()
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

TEST_F(SimulateTest, TickNoEffect)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 3 1 2 0",
        "VAMPIRE 3 9 9 3 1 2 0",
        "VAMPIRE 4 1 9 3 1 2 0",
        "VAMPIRE 2 9 1 3 1 2 0",
        "BAT1 4 1 5 1 6 1 3 2 7 2 2 3 3 3 7 3 8 3 1 4 9 4 1 5 9 5 1 6 9 6 2 7 3 7 7 7 8 7 3 8 7 8 4 9 5 9 6 9",
        "BAT2 5 2 4 3 6 3 3 4 7 4 2 5 8 5 3 6 7 6 4 7 6 7 5 8",
        "BAT3 5 3 5 4 3 5 4 5 5 5 6 5 7 5 5 6 5 7",
    };
    // clang-format on
    TickDescription state = parseTickDescription(info);
    mSimulator->SetState(state);
    const auto [newState, newPoints] = mSimulator->Tick();
    state.mRequest.mTick++;
    ASSERT_EQ(state, newState);
    ASSERT_EQ(newPoints.at(1), 0);
    ASSERT_EQ(newPoints.at(3), 0);
    ASSERT_EQ(newPoints.at(4), 0);
    ASSERT_EQ(newPoints.at(2), 0);
}

TEST_F(SimulateTest, TickDecreases)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "GRENADE 1 1 3 5 2",
        "GRENADE 1 9 3 3 2",
        "POWERUP TOMATO -3 2 1",
        "POWERUP TOMATO -1 2 1",
        "POWERUP GRENADE 2 3 1",
        "POWERUP BATTERY 10 7 1",
        "POWERUP SHOE -10 8 1"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades[0].mTick, 4);
    ASSERT_EQ(newState.mGrenades[1].mTick, 2);
    ASSERT_EQ(newState.mPowerUps[0].mRemainingTick, -2);
    ASSERT_EQ(newState.mPowerUps[1].mRemainingTick, 10);
    ASSERT_EQ(newState.mPowerUps[2].mRemainingTick, 1);
    ASSERT_EQ(newState.mPowerUps[3].mRemainingTick, 9);
    ASSERT_EQ(newState.mPowerUps[4].mRemainingTick, -9);
}

TEST_F(SimulateTest, PowerupDisappear)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "POWERUP TOMATO -3 2 1",
        "POWERUP TOMATO -1 2 1",
        "POWERUP TOMATO 1 2 1",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 2);
    ASSERT_EQ(newState.mPowerUps[0].mRemainingTick, -2);
    ASSERT_EQ(newState.mPowerUps[1].mRemainingTick, 10);
}

TEST_F(SimulateTest, PowerupPickupSingleMeTomatoNoEffect)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 3 1 2 0",
        "POWERUP TOMATO 3 2 1",
        "POWERUP TOMATO 3 1 1",
        "POWERUP TOMATO 5 3 1",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 2);
    ASSERT_EQ(newState.mPowerUps[0].mRemainingTick, 2);
    ASSERT_EQ(newState.mPowerUps[1].mRemainingTick, 4);
    ASSERT_EQ(newState.mMe.mHealth, 3);
    ASSERT_EQ(newPoints.at(1), 48);
}

TEST_F(SimulateTest, PowerupNoPickupOnCountdown)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 1 1 2 0",
        "POWERUP TOMATO -3 1 1",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 1);
    ASSERT_EQ(newState.mPowerUps[0].mRemainingTick, -2);
    ASSERT_EQ(newState.mMe.mHealth, 1);
    ASSERT_EQ(newPoints.at(1), 0);
}

TEST_F(SimulateTest, PowerupPickupSingleMeTomato)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 1 1 2 0",
        "POWERUP TOMATO 3 2 1",
        "POWERUP TOMATO 3 1 1",
        "POWERUP TOMATO 5 3 1",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 2);
    ASSERT_EQ(newState.mPowerUps[0].mRemainingTick, 2);
    ASSERT_EQ(newState.mPowerUps[1].mRemainingTick, 4);
    ASSERT_EQ(newState.mMe.mHealth, 2);
    ASSERT_EQ(newPoints.at(1), 48);
}

TEST_F(SimulateTest, PowerupPickupMeWithOtherTomato)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 1 1 2 0",
        "VAMPIRE 2 1 1 2 1 2 0",
        "POWERUP TOMATO 3 2 1",
        "POWERUP TOMATO 3 1 1",
        "POWERUP TOMATO 5 3 1",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 2);
    ASSERT_EQ(newState.mPowerUps[0].mRemainingTick, 2);
    ASSERT_EQ(newState.mPowerUps[1].mRemainingTick, 4);
    ASSERT_EQ(newState.mMe.mHealth, 2);
    ASSERT_EQ(newState.mEnemyVampires[0].mHealth, 3);
    ASSERT_EQ(newPoints.at(1), 48);
    ASSERT_EQ(newPoints.at(2), 48);
}

TEST_F(SimulateTest, PowerupPickupOthersTomato)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 4 1 1 1 2 0",
        "VAMPIRE 2 1 1 2 1 2 0",
        "VAMPIRE 3 1 1 1 1 2 0",
        "POWERUP TOMATO 3 2 1",
        "POWERUP TOMATO 3 1 1",
        "POWERUP TOMATO 5 3 1",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 2);
    ASSERT_EQ(newState.mPowerUps[0].mRemainingTick, 2);
    ASSERT_EQ(newState.mPowerUps[1].mRemainingTick, 4);
    ASSERT_EQ(newState.mMe.mHealth, 1);
    ASSERT_EQ(newState.mEnemyVampires[0].mHealth, 3);
    ASSERT_EQ(newState.mEnemyVampires[1].mHealth, 2);
    ASSERT_EQ(newPoints.at(1), 0);
    ASSERT_EQ(newPoints.at(2), 48);
    ASSERT_EQ(newPoints.at(3), 48);
}

TEST_F(SimulateTest, PowerupPickupOthersTomato2)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 4 1 1 1 2 0",
        "VAMPIRE 2 1 1 2 1 2 0",
        "VAMPIRE 3 1 1 3 1 2 0",
        "POWERUP TOMATO 3 2 1",
        "POWERUP TOMATO 3 1 1",
        "POWERUP TOMATO 5 3 1",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 2);
    ASSERT_EQ(newState.mPowerUps[0].mRemainingTick, 2);
    ASSERT_EQ(newState.mPowerUps[1].mRemainingTick, 4);
    ASSERT_EQ(newState.mMe.mHealth, 1);
    ASSERT_EQ(newState.mEnemyVampires[0].mHealth, 3);
    ASSERT_EQ(newState.mEnemyVampires[1].mHealth, 3);
    ASSERT_EQ(newPoints.at(1), 0);
    ASSERT_EQ(newPoints.at(2), 48);
    ASSERT_EQ(newPoints.at(3), 48);
}

TEST_F(SimulateTest, PowerupPickupGrenade)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 1 1 2 0",
        "POWERUP GRENADE 3 1 1",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 0);
    ASSERT_EQ(newState.mMe.mPlacableGrenades, 2);
    ASSERT_EQ(newPoints.at(1), 48);
}

TEST_F(SimulateTest, PowerupPickupBattery)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 1 1 2 0",
        "POWERUP BATTERY 3 1 1",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 0);
    ASSERT_EQ(newState.mMe.mGrenadeRange, 3);
    ASSERT_EQ(newPoints.at(1), 48);
}

TEST_F(SimulateTest, PowerupPickupShoe)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 1 1 2 0",
        "POWERUP SHOE 3 1 1",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 0);
    ASSERT_EQ(newState.mMe.mRunningShoesTick, 22);
    ASSERT_EQ(newPoints.at(1), 48);
}

TEST_F(SimulateTest, PowerupPickupShoeAddition)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 1 1 2 7",
        "POWERUP SHOE 3 1 1",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 0);
    ASSERT_EQ(newState.mMe.mRunningShoesTick, 28);
    ASSERT_EQ(newPoints.at(1), 48);
}

TEST_F(SimulateTest, BlowUpSingleGrenade)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "GRENADE 1 3 3 1 2"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
}

TEST_F(SimulateTest, BlowUpTwoGrenadeNoReaction)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "GRENADE 1 2 1 1 2",
        "GRENADE 1 1 2 2 2"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 1);
    ASSERT_EQ(newState.mGrenades[0].mTick, 1);
}

TEST_F(SimulateTest, BlowUpChainReaction)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "GRENADE 1 3 3 1 2",
        "GRENADE 1 3 5 5 2"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
}

TEST_F(SimulateTest, BlowUpChainReaction2)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "GRENADE 1 3 6 1 5",
        "GRENADE 2 3 6 1 5",
        "GRENADE 1 3 6 2 5",
        "GRENADE 2 3 6 2 5",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
}

TEST_F(SimulateTest, BlowUpGrenadeKillBats)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "GRENADE 1 3 3 1 2",
        "BAT1 1 3 1 4",
        "BAT2 4 3 5 3",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
    ASSERT_EQ(newState.mAllBats.size(), 3);
    ASSERT_EQ(newState.mBat1.size(), 2);
    ASSERT_EQ(newState.mBat2.size(), 1);
    ASSERT_EQ(newState.mBat1[0].mX, 4);
    ASSERT_EQ(newState.mBat1[0].mY, 1);
    ASSERT_EQ(newState.mBat1[1].mX, 3);
    ASSERT_EQ(newState.mBat1[1].mY, 4);
    ASSERT_EQ(newState.mBat2[0].mX, 3);
    ASSERT_EQ(newState.mBat2[0].mY, 5);
    ASSERT_EQ(newPoints.at(1), 24.F);
}

TEST_F(SimulateTest, BlowUpGrenadeKillVampires)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 3 1 2 1 2 0",
        "GRENADE 1 3 3 1 2"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
    ASSERT_EQ(newState.mMe.mHealth, 1);
    ASSERT_EQ(newPoints.at(1), -48);
}

TEST_F(SimulateTest, BlowUpGrenadeTreesProtect)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 2 2 1 2 0",
        "GRENADE 1 3 2 1 2",
        "BAT1 5 2"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
    ASSERT_EQ(newState.mMe.mHealth, 2);
    ASSERT_EQ(newState.mBat1.size(), 1);
    ASSERT_EQ(newState.mBat1[0].mX, 2);
    ASSERT_EQ(newState.mBat1[0].mY, 5);
    ASSERT_EQ(newPoints.at(1), 0);
}

TEST_F(SimulateTest, BlowUpGrenadeChainReactionKills)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 5 2 1 2 0",
        "GRENADE 1 1 1 1 2",
        "GRENADE 1 1 3 5 2"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
    ASSERT_EQ(newState.mMe.mHealth, 1);
    ASSERT_EQ(newPoints.at(1), -48);
}

TEST_F(SimulateTest, BlowUpGrenadeInterfereChainReactionKills)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 2 2 1 2 0",
        "GRENADE 1 1 1 1 2",
        "GRENADE 1 1 3 5 2"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
    ASSERT_EQ(newState.mMe.mHealth, 1);
    ASSERT_EQ(newPoints.at(1), -48);
}

TEST_F(SimulateTest, OneBatTwoGrenade)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 5 2 1 2 0",
        "VAMPIRE 2 5 5 2 1 2 0",
        "BAT1 1 1",
        "GRENADE 1 1 2 1 2",
        "GRENADE 2 2 1 1 2",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newPoints.at(1), 12);
    ASSERT_EQ(newPoints.at(2), 12);
}

TEST_F(SimulateTest, OneBatTwoGrenadeSamePoisition)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 5 2 1 2 0",
        "VAMPIRE 2 5 5 2 1 2 0",
        "BAT1 1 1",
        "GRENADE 1 1 2 1 2",
        "GRENADE 2 1 2 1 2",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newPoints.at(1), 6);
    ASSERT_EQ(newPoints.at(2), 6);
}

TEST_F(SimulateTest, InjureOneVampireTwoGrenade)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 5 2 1 2 0",
        "VAMPIRE 2 5 5 2 1 2 0",
        "VAMPIRE 3 1 1 2 1 2 0",
        "GRENADE 1 1 2 1 2",
        "GRENADE 2 2 1 1 2",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newPoints.at(1), 48 / 2);
    ASSERT_EQ(newPoints.at(2), 48 / 2);
    ASSERT_EQ(newPoints.at(3), 0);
}

TEST_F(SimulateTest, InjureOneVampireTwoGrenadeChain)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 5 2 1 2 0",
        "VAMPIRE 2 5 5 2 1 2 0",
        "VAMPIRE 3 1 1 2 1 2 0",
        "GRENADE 1 1 2 1 2",
        "GRENADE 2 1 4 3 2",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newPoints.at(1), 48);
    ASSERT_EQ(newPoints.at(2), 0);
    ASSERT_EQ(newPoints.at(3), 0);
    ASSERT_TRUE(newState.mGrenades.empty());
}

TEST_F(SimulateTest, InjureOneVampireTwoGrenadeChain2)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 5 2 1 2 0",
        "VAMPIRE 2 5 5 2 1 2 0",
        "VAMPIRE 3 1 1 2 1 2 0",
        "GRENADE 1 1 2 1 2",
        "GRENADE 2 1 3 3 2",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newPoints.at(1), 48 / 2);
    ASSERT_EQ(newPoints.at(2), 48 / 2);
    ASSERT_EQ(newPoints.at(3), 0);
    ASSERT_TRUE(newState.mGrenades.empty());
}

TEST_F(SimulateTest, InjureOneVampireTwoGrenadeChain3)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 5 2 1 2 0",
        "VAMPIRE 2 5 5 2 1 2 0",
        "VAMPIRE 3 1 1 2 1 2 0",
        "GRENADE 1 1 3 1 2",
        "GRENADE 2 3 3 3 2",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newPoints.at(1), 48);
    ASSERT_EQ(newPoints.at(2), 0);
    ASSERT_EQ(newPoints.at(3), 0);
    ASSERT_TRUE(newState.mGrenades.empty());
}

TEST_F(SimulateTest, KillOneVampireTwoGrenade)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 5 2 1 2 0",
        "VAMPIRE 2 5 5 2 1 2 0",
        "VAMPIRE 3 1 1 1 1 2 0",
        "GRENADE 1 1 2 1 2",
        "GRENADE 2 2 1 1 2",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newPoints.at(1), (48 + 96) / 2);
    ASSERT_EQ(newPoints.at(2), (48 + 96) / 2);
    ASSERT_EQ(newPoints.at(3), 0);
}
TEST_F(SimulateTest, OneBatTwoGrenadeChain)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 5 2 1 2 0",
        "VAMPIRE 2 5 5 2 1 2 0",
        "BAT1 1 1",
        "GRENADE 1 1 2 1 2",
        "GRENADE 2 1 3 1 2",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newPoints.at(1), 12 / 2);
    ASSERT_EQ(newPoints.at(2), 12 / 2);
}

TEST_F(SimulateTest, OneBatTwoGrenadeChainMultiplePoints)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 5 2 1 2 0",
        "VAMPIRE 2 5 5 2 1 2 0",
        "BAT1 1 1",
        "GRENADE 1 1 2 1 2",
        "GRENADE 2 1 3 1 2",
        "GRENADE 1 2 1 1 2",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newPoints.at(1), 12 + 12 / 2);
    ASSERT_EQ(newPoints.at(2), 12 / 2);
}

TEST_F(SimulateTest, BlowUpGrenadeKillVampiresFinally)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 5 2 1 2 0",
        "VAMPIRE 2 3 1 1 1 2 0",
        "GRENADE 1 3 3 1 2"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
    ASSERT_EQ(newState.mMe.mHealth, 2);
    ASSERT_EQ(newState.mEnemyVampires.size(), 0);
    ASSERT_EQ(newPoints.at(1), 48 + 96);
    ASSERT_EQ(newPoints.at(2), 0);
}
TEST_F(SimulateTest, GhostMode)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 3 2 2 1 2 0",
        "VAMPIRE 2 3 1 2 1 2 0",
        "GRENADE 1 3 3 1 2"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
    ASSERT_EQ(newState.mMe.mGhostModeTick, 3);
    ASSERT_EQ(newState.mEnemyVampires[0].mGhostModeTick, 3);
    ASSERT_EQ(newPoints.at(1), 0); // -48 +48
    ASSERT_EQ(newPoints.at(2), 0);

    mSimulator->SetState(newState);
    const auto [finalState, finalPoints] = mSimulator->Tick();
    ASSERT_EQ(finalState.mMe.mGhostModeTick, 2);
    ASSERT_EQ(finalState.mEnemyVampires[0].mGhostModeTick, 2);

    ASSERT_EQ(finalPoints.at(1), 0);
    ASSERT_EQ(finalPoints.at(2), 0);
}

TEST_F(SimulateTest, GhostModeProtection)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 0",
        "GRENADE 1 1 2 1 2",
        "GRENADE 1 2 1 2 2"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 1);
    ASSERT_EQ(newState.mMe.mGhostModeTick, 3);
    ASSERT_EQ(newState.mMe.mHealth, 1);
    mSimulator->SetState(newState);
    ASSERT_EQ(newPoints.at(1), -48);

    const auto [finalState, finalPoints] = mSimulator->Tick();
    ASSERT_EQ(finalState.mGrenades.size(), 0);
    ASSERT_EQ(finalState.mMe.mGhostModeTick, 2);
    ASSERT_EQ(finalState.mMe.mHealth, 1);
    ASSERT_EQ(finalPoints.at(1), 0);
}

TEST_F(SimulateTest, Plantgrenade)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 0",
        "VAMPIRE 2 3 1 2 1 2 0",
        "VAMPIRE 3 5 1 2 1 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    mSimulator->SetVampireMove(1, { true, {} });
    mSimulator->SetVampireMove(2, { true, {} });
    mSimulator->SetVampireMove(3, { false, {} });
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 2);
    ASSERT_EQ(newState.mGrenades[0].mX, 1);
    ASSERT_EQ(newState.mGrenades[0].mY, 1);
    ASSERT_EQ(newState.mGrenades[0].mTick, 5);
    ASSERT_EQ(newState.mGrenades[1].mX, 1);
    ASSERT_EQ(newState.mGrenades[1].mY, 3);
    ASSERT_EQ(newState.mGrenades[1].mTick, 5);
    ASSERT_EQ(newState.mMe.mPlacableGrenades, 0);
    ASSERT_EQ(newState.mEnemyVampires[0].mPlacableGrenades, 0);
    ASSERT_EQ(newState.mEnemyVampires[1].mPlacableGrenades, 1);

    ASSERT_EQ(newPoints.at(1), 0);
    ASSERT_EQ(newPoints.at(2), 0);
    ASSERT_EQ(newPoints.at(3), 0);
}

TEST_F(SimulateTest, PlacableGrenadeRegain)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 0 2 0",
        "VAMPIRE 2 1 1 2 0 2 0",
        "GRENADE 1 3 3 1 1",
        "GRENADE 2 3 3 1 1"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);

    ASSERT_EQ(tick.mMe.mPlacableGrenades, 0);
    ASSERT_EQ(tick.mEnemyVampires.front().mPlacableGrenades, 0);

    const auto [newState, newPoints] = mSimulator->Tick();

    ASSERT_EQ(newState.mMe.mPlacableGrenades, 1);
    ASSERT_EQ(newState.mEnemyVampires.front().mPlacableGrenades, 1);
}

TEST_F(SimulateTest, PlantgrenadeAfterDeath)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "GRENADE 1 1 2 1 2",
        "VAMPIRE 1 1 1 1 1 2 0",
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    mSimulator->SetVampireMove(1, { true, {} });
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
}

TEST_F(SimulateTest, StepSimple)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    mSimulator->SetVampireMove(1, { true, { 'R', 'R' } });
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mMe.mX, 3);
    ASSERT_EQ(newState.mMe.mY, 1);
    ASSERT_EQ(newPoints.at(1), 0);
}

TEST_F(SimulateTest, StepSimpleAfterDeath)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "GRENADE 1 1 2 1 2",
        "VAMPIRE 1 1 1 2 1 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    mSimulator->SetVampireMove(1, { true, { 'R', 'R' } });
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mMe.mX, 3);
    ASSERT_EQ(newState.mMe.mY, 1);
}

TEST_F(SimulateTest, StepExtra)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 2"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    mSimulator->SetVampireMove(1, { false, { 'R', 'R', 'R' } });
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mMe.mX, 4);
    ASSERT_EQ(newState.mMe.mY, 1);
    ASSERT_EQ(newPoints.at(1), 0);
}

TEST_F(SimulateTest, StepToTree)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    mSimulator->SetVampireMove(1, { false, { 'R', 'D' } });
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mMe.mX, 2);
    ASSERT_EQ(newState.mMe.mY, 1);
    ASSERT_EQ(newPoints.at(1), 0);
}

TEST_F(SimulateTest, StepToGrenade)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 0",
        "GRENADE 1 1 3 5"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    mSimulator->SetVampireMove(1, { false, { 'R', 'R' } });
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mMe.mX, 2);
    ASSERT_EQ(newState.mMe.mY, 1);
    ASSERT_EQ(newPoints.at(1), 0);
}

TEST_F(SimulateTest, StepToBat)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 0",
        "BAT1 1 3"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    mSimulator->SetVampireMove(1, { false, { 'R', 'R' } });
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mMe.mX, 2);
    ASSERT_EQ(newState.mMe.mY, 1);
    ASSERT_EQ(newPoints.at(1), 0);
}

TEST_F(SimulateTest, StepBackToGrenade)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    mSimulator->SetVampireMove(1, { true, { 'R', 'L' } });
    const auto [newState, newPoints] = mSimulator->Tick();
    ASSERT_EQ(newState.mMe.mX, 2);
    ASSERT_EQ(newState.mMe.mY, 1);
    ASSERT_EQ(newPoints.at(1), 0);
}

TEST_F(SimulateTest, TrivialBadBombPlant)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    ASSERT_TRUE(mSimulator->IsValidMove(1, { true, {} }));
    mSimulator->SetVampireMove(1, { true, {} });
    const auto [newState, newPoints] = mSimulator->Tick();
    mSimulator->SetState(newState);
    ASSERT_FALSE(mSimulator->IsValidMove(1, { true, {} }));
    ASSERT_EQ(newPoints.at(1), 0);
}

TEST_F(SimulateTest, TrivialTooManySteps)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    ASSERT_FALSE(mSimulator->IsValidMove(1, { true, { 'D', 'U', 'D' } }));
}

TEST_F(SimulateTest, TrivialTooManySteps2)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 5"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    ASSERT_TRUE(mSimulator->IsValidMove(1, { false, { 'D', 'U', 'D' } }));
}

TEST_F(SimulateTest, TrivialWall)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    ASSERT_FALSE(mSimulator->IsValidMove(1, { false, { 'R', 'D' } }));
    ASSERT_FALSE(mSimulator->IsValidMove(1, { false, { 'L', 'L' } }));
    ASSERT_TRUE(mSimulator->IsValidMove(1, { false, { 'R', 'R' } }));
}

TEST_F(SimulateTest, GetBlowAreasDisjunct)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "GRENADE 1 1 1 2 2",
        "GRENADE 2 5 5 4 3"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const std::vector<Simulator::BlowArea> areas = mSimulator->GetBlowAreas();
    ASSERT_EQ(areas.size(), 2);
    const auto tick2It = std::find_if(areas.begin(), areas.end(), [](const auto& area) { return area.mTickCount == 2; });
    ASSERT_TRUE(tick2It != areas.end());
    ASSERT_EQ(tick2It->mTickCount, 2);
    ASSERT_EQ(tick2It->mVampireIds.size(), 1);
    ASSERT_THAT(tick2It->mVampireIds, testing::UnorderedElementsAre(1));
    ASSERT_THAT(tick2It->mArea.getAsVector(),
        testing::UnorderedElementsAre(std::pair<int, int> { 1, 1 }, std::pair<int, int> { 1, 2 }, std::pair<int, int> { 1, 3 }, std::pair<int, int> { 2, 1 },
            std::pair<int, int> { 3, 1 }));
    const auto tick4It = std::find_if(areas.begin(), areas.end(), [](const auto& area) { return area.mTickCount == 4; });
    ASSERT_TRUE(tick4It != areas.end());
    ASSERT_EQ(tick4It->mTickCount, 4);
    ASSERT_EQ(tick4It->mVampireIds.size(), 1);
    ASSERT_THAT(tick4It->mVampireIds, testing::UnorderedElementsAre(2));
    ASSERT_THAT(tick4It->mArea.getAsVector(),
        testing::UnorderedElementsAre(std::pair<int, int> { 5, 5 }, std::pair<int, int> { 4, 5 }, std::pair<int, int> { 3, 5 }, std::pair<int, int> { 2, 5 },
            std::pair<int, int> { 6, 5 }, std::pair<int, int> { 7, 5 }, std::pair<int, int> { 8, 5 }, std::pair<int, int> { 5, 4 },
            std::pair<int, int> { 5, 3 }, std::pair<int, int> { 5, 2 }, std::pair<int, int> { 5, 6 }, std::pair<int, int> { 5, 7 },
            std::pair<int, int> { 5, 8 }));
}

TEST_F(SimulateTest, GetBlowAreasJoint)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "GRENADE 1 3 1 2 2",
        "GRENADE 2 3 3 4 3"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const std::vector<Simulator::BlowArea> areas = mSimulator->GetBlowAreas();
    ASSERT_EQ(areas.size(), 1);
    ASSERT_EQ(areas[0].mTickCount, 2);
    ASSERT_EQ(areas[0].mVampireIds.size(), 2);
    ASSERT_THAT(areas[0].mVampireIds, testing::UnorderedElementsAre(1, 2));
    ASSERT_THAT(areas[0].mArea.getAsVector(),
        testing::UnorderedElementsAre(std::pair<int, int> { 1, 1 }, std::pair<int, int> { 1, 2 }, std::pair<int, int> { 1, 3 }, std::pair<int, int> { 1, 4 },
            std::pair<int, int> { 1, 5 }, std::pair<int, int> { 2, 3 }, std::pair<int, int> { 3, 1 }, std::pair<int, int> { 3, 2 },
            std::pair<int, int> { 3, 3 }, std::pair<int, int> { 3, 4 }, std::pair<int, int> { 3, 5 }, std::pair<int, int> { 3, 6 },
            std::pair<int, int> { 4, 3 }, std::pair<int, int> { 5, 3 }, std::pair<int, int> { 6, 3 }));
}

TEST_F(SimulateTest, LightStart)
{
    // clang-format off
    mSimulator->SetState(parseTickDescription({
        "REQ 775 500 1"
    }));
    // clang-format on

    EXPECT_TRUE(mSimulator->GetLitArea().find(0, 0));
}

TEST_F(SimulateTest, LightEndCrash)
{
    // clang-format off
    mSimulator->SetState(parseTickDescription({
        "REQ 775 800 1"
    }));
    // clang-format on

    EXPECT_EQ(mSimulator->GetLitArea().getAsVector().size(), 11 * 11);
}

TEST_F(SimulateTest, DecreaseHpByLight)
{
    std::pair<TickDescription, Simulator::NewPoints> tickRes;

    // clang-format off
    mSimulator->SetState(parseTickDescription({
        "REQ 775 509 1",
        "VAMPIRE 1 1 1 2 1 2 0"
    }));
    // clang-format on
    tickRes = mSimulator->Tick();
    ASSERT_EQ(tickRes.first.mMe.mHealth, 2);

    // clang-format off
    mSimulator->SetState(parseTickDescription({
        "REQ 775 510 1",
        "VAMPIRE 1 1 1 2 1 2 0"
    }));
    // clang-format on
    tickRes = mSimulator->Tick();
    ASSERT_EQ(tickRes.first.mMe.mHealth, 1);

    // clang-format off
    mSimulator->SetState(parseTickDescription({
        "REQ 775 510 1",
        "VAMPIRE 1 2 1 2 1 2 0"
    }));
    // clang-format on
    tickRes = mSimulator->Tick();
    ASSERT_EQ(tickRes.first.mMe.mHealth, 2);

    // clang-format off
    mSimulator->SetState(parseTickDescription({
        "REQ 775 516 1",
        "VAMPIRE 1 1 2 2 1 2 0"
    }));
    // clang-format on
    tickRes = mSimulator->Tick();
    ASSERT_EQ(tickRes.first.mMe.mHealth, 2);

    // clang-format off
    mSimulator->SetState(parseTickDescription({
        "REQ 775 517 1",
        "VAMPIRE 1 1 2 2 1 2 0"
    }));
    // clang-format on
    tickRes = mSimulator->Tick();
    ASSERT_EQ(tickRes.first.mMe.mHealth, 1);
}