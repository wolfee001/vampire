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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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
    const TickDescription tick = parseTickDescription(info);
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

TEST_F(SimulateTest, Plantgrenade)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 0",
        "VAMPIRE 2 1 3 2 1 2 0",
        "VAMPIRE 3 1 5 2 1 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    mSimulator->SetVampireMove(1, { true, {} });
    mSimulator->SetVampireMove(2, { true, {} });
    mSimulator->SetVampireMove(3, { false, {} });
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mGrenades.size(), 2);
    ASSERT_EQ(newState.mGrenades[0].mX, 1);
    ASSERT_EQ(newState.mGrenades[0].mY, 1);
    ASSERT_EQ(newState.mGrenades[0].mTick, 5);
    ASSERT_EQ(newState.mGrenades[1].mX, 1);
    ASSERT_EQ(newState.mGrenades[1].mY, 3);
    ASSERT_EQ(newState.mGrenades[1].mTick, 5);
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
    const TickDescription newState = mSimulator->Tick();
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
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mMe.mX, 4);
    ASSERT_EQ(newState.mMe.mY, 1);
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
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mMe.mX, 2);
    ASSERT_EQ(newState.mMe.mY, 1);
}

TEST_F(SimulateTest, StepToGrenade)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 0",
        "GRENADE 1 3 1 5"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    mSimulator->SetVampireMove(1, { false, { 'R', 'R' } });
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mMe.mX, 2);
    ASSERT_EQ(newState.mMe.mY, 1);
}

TEST_F(SimulateTest, StepToBat)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 1 1 2 1 2 0",
        "BAT1 3 1"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    mSimulator->SetVampireMove(1, { false, { 'R', 'R' } });
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mMe.mX, 2);
    ASSERT_EQ(newState.mMe.mY, 1);
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
    const TickDescription newState = mSimulator->Tick();
    ASSERT_EQ(newState.mMe.mX, 2);
    ASSERT_EQ(newState.mMe.mY, 1);
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
    const TickDescription newState = mSimulator->Tick();
    mSimulator->SetState(newState);
    ASSERT_FALSE(mSimulator->IsValidMove(1, { true, {} }));
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
    ASSERT_EQ(areas[0].mTickCount, 2);
    ASSERT_EQ(areas[0].mVampireIds.size(), 1);
    ASSERT_THAT(areas[0].mVampireIds, testing::ElementsAre(1));
    ASSERT_EQ(areas[0].mArea, (std::set<std::pair<int, int>> { { 1, 1 }, { 1, 2 }, { 1, 3 }, { 2, 1 }, { 3, 1 } }));
    ASSERT_EQ(areas[1].mTickCount, 4);
    ASSERT_EQ(areas[1].mVampireIds.size(), 1);
    ASSERT_THAT(areas[1].mVampireIds, testing::ElementsAre(2));
    ASSERT_EQ(areas[1].mArea,
        (std::set<std::pair<int, int>> {
            { 5, 5 },
            { 4, 5 },
            { 3, 5 },
            { 2, 5 },
            { 6, 5 },
            { 7, 5 },
            { 8, 5 },
            { 5, 4 },
            { 5, 3 },
            { 5, 2 },
            { 5, 6 },
            { 5, 7 },
            { 5, 8 },
        }));
}

TEST_F(SimulateTest, GetBlowAreasJoint)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "GRENADE 1 1 3 2 2",
        "GRENADE 2 3 3 4 3"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    mSimulator->SetState(tick);
    const std::vector<Simulator::BlowArea> areas = mSimulator->GetBlowAreas();
    ASSERT_EQ(areas.size(), 1);
    ASSERT_EQ(areas[0].mTickCount, 2);
    ASSERT_EQ(areas[0].mVampireIds.size(), 2);
    ASSERT_THAT(areas[0].mVampireIds, testing::ElementsAre(1, 2));
    ASSERT_EQ(areas[0].mArea,
        (std::set<std::pair<int, int>> {
            { 1, 1 },
            { 1, 2 },
            { 1, 3 },
            { 1, 4 },
            { 1, 5 },
            { 2, 3 },
            { 3, 1 },
            { 3, 2 },
            { 3, 3 },
            { 3, 4 },
            { 3, 5 },
            { 3, 6 },
            { 4, 3 },
            { 5, 3 },
            { 6, 3 },
        }));
}