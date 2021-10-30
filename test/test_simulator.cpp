#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "../simulator.h"
#include "../solver.h"

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
        mGameDescripton = solver::parseGameDescription(startInfo);

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
    TickDescription state = solver::parseTickDescription(info);
    mSimulator->SetState(state);
    const TickDescription newState = mSimulator->Tick();
    state.mRequest.mTick++;
    ASSERT_EQ(state, newState);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 2);
    ASSERT_EQ(newState.mPowerUps[0].mRemainingTick, 2);
    ASSERT_EQ(newState.mPowerUps[1].mRemainingTick, 4);
    ASSERT_EQ(newState.mMe.mHealth, 3);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 2);
    ASSERT_EQ(newState.mPowerUps[0].mRemainingTick, 2);
    ASSERT_EQ(newState.mPowerUps[1].mRemainingTick, 4);
    ASSERT_EQ(newState.mMe.mHealth, 2);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 2);
    ASSERT_EQ(newState.mPowerUps[0].mRemainingTick, 2);
    ASSERT_EQ(newState.mPowerUps[1].mRemainingTick, 4);
    ASSERT_EQ(newState.mMe.mHealth, 2);
    ASSERT_EQ(newState.mEnemyVampires[0].mHealth, 3);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 2);
    ASSERT_EQ(newState.mPowerUps[0].mRemainingTick, 2);
    ASSERT_EQ(newState.mPowerUps[1].mRemainingTick, 4);
    ASSERT_EQ(newState.mMe.mHealth, 1);
    ASSERT_EQ(newState.mEnemyVampires[0].mHealth, 3);
    ASSERT_EQ(newState.mEnemyVampires[1].mHealth, 2);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 2);
    ASSERT_EQ(newState.mPowerUps[0].mRemainingTick, 2);
    ASSERT_EQ(newState.mPowerUps[1].mRemainingTick, 4);
    ASSERT_EQ(newState.mMe.mHealth, 1);
    ASSERT_EQ(newState.mEnemyVampires[0].mHealth, 3);
    ASSERT_EQ(newState.mEnemyVampires[1].mHealth, 3);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 0);
    ASSERT_EQ(newState.mMe.mPlacableGrenades, 2);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 0);
    ASSERT_EQ(newState.mMe.mGrenadeRange, 3);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 0);
    ASSERT_EQ(newState.mMe.mRunningShoesTick, 22);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mPowerUps.size(), 0);
    ASSERT_EQ(newState.mMe.mRunningShoesTick, 28);
}

TEST_F(SimulateTest, BlowUpSingleGrenade)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "GRENADE 1 3 3 1 2"
    };
    // clang-format on
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
}

TEST_F(SimulateTest, BlowUpGrenadeKillBats)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "GRENADE 1 3 3 1 2",
        "BAT1 3 1 4 1",
        "BAT2 3 4 3 5",
    };
    // clang-format on
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
    ASSERT_EQ(newState.mMe.mHealth, 1);
}

TEST_F(SimulateTest, BlowUpGrenadeTreesProtect)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 2 1 2 1 2 0",
        "GRENADE 1 2 3 1 2",
        "BAT1 2 5"
    };
    // clang-format on
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
    ASSERT_EQ(newState.mMe.mHealth, 2);
    ASSERT_EQ(newState.mBat1.size(), 1);
    ASSERT_EQ(newState.mBat1[0].mX, 2);
    ASSERT_EQ(newState.mBat1[0].mY, 5);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
    ASSERT_EQ(newState.mMe.mHealth, 1);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
    ASSERT_EQ(newState.mMe.mHealth, 1);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
    ASSERT_EQ(newState.mMe.mHealth, 2);
    ASSERT_EQ(newState.mEnemyVampires.size(), 0);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 0);
    ASSERT_EQ(newState.mMe.mGhostModeTick, 3);
    ASSERT_EQ(newState.mEnemyVampires[0].mGhostModeTick, 3);
    mSimulator->SetState(newState);
    const TickDescription finalState = mSimulator->Tick();
    ASSERT_EQ(finalState.mMe.mGhostModeTick, 2);
    ASSERT_EQ(finalState.mEnemyVampires[0].mGhostModeTick, 2);
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
    const TickDescription tick = solver::parseTickDescription(info);
    mSimulator->SetState(tick);
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 1);
    ASSERT_EQ(newState.mMe.mGhostModeTick, 3);
    ASSERT_EQ(newState.mMe.mHealth, 1);
    mSimulator->SetState(newState);
    const TickDescription finalState = mSimulator->Tick();
    ASSERT_EQ(finalState.mGrenades.size(), 0);
    ASSERT_EQ(finalState.mMe.mGhostModeTick, 2);
    ASSERT_EQ(finalState.mMe.mHealth, 1);
}

// TEST_F(SimulateTest, NONONONONOOOOOOO)
// {
//     // clang-format off
//     std::vector<std::string> info = {
//         "REQ 775 0 1",
//         "VAMPIRE 1 1 1 3 1 2 0",
//         "VAMPIRE 3 9 9 3 1 2 0",
//         "VAMPIRE 4 1 9 3 1 2 0",
//         "VAMPIRE 2 9 1 3 1 2 0",
//         "BAT1 4 1 5 1 6 1 3 2 7 2 2 3 3 3 7 3 8 3 1 4 9 4 1 5 9 5 1 6 9 6 2 7 3 7 7 7 8 7 3 8 7 8 4 9 5 9 6 9",
//         "BAT2 5 2 4 3 6 3 3 4 7 4 2 5 8 5 3 6 7 6 4 7 6 7 5 8",
//         "BAT3 5 3 5 4 3 5 4 5 5 5 6 5 7 5 5 6 5 7",
//         "GRENADE 1 1 3 15 2",
//         "GRENADE 1 9 3 0 2",
//         "POWERUP TOMATO -3 2 1",
//         "POWERUP GRENADE 2 3 1",
//         "POWERUP BATTERY 10 7 1",
//         "POWERUP SHOE -10 8 1"
//     };
//     // clang-format on
//     const TickDescription tick = solver::parseTickDescription(info);
//     mSimulator->SetState(tick);
// }