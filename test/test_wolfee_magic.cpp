#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../parser.h"
#include "../wolfee_magic.h"

class WolfeeMagicTest : public testing::Test {
public:
    WolfeeMagicTest()
    {
        std::vector<std::string> startInfo = {
            "MESSAGE OK",
            "LEVEL 1",
            "GAMEID 775",
            "TEST 1",
            "MAXTICK 500",
            "GRENADERADIUS 2",
            "SIZE 21",
        };
        mGameDescripton = parseGameDescription(startInfo);

        mSimulator = std::make_unique<Simulator>(mGameDescripton);
    }

protected:
    GameDescription mGameDescripton;
    std::unique_ptr<Simulator> mSimulator;
};

TEST_F(WolfeeMagicTest, RejectTriviallyWrong)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 3 1 2 2 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    std::vector<pos_t> path = { { 3, 1 }, { 3, 2 }, { 3, 3 }, { 2, 3 } };
    std::vector<pos_t> retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -10, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>());
}

TEST_F(WolfeeMagicTest, Test1)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 4 3 2 2 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    std::vector<pos_t> path = { { 4, 3 }, { 3, 3 }, { 2, 3 } };
    std::vector<pos_t> retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -6, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 4, 3 } }));
}

TEST_F(WolfeeMagicTest, Test2)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 6 3 2 2 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    std::vector<pos_t> path = { { 6, 3 }, { 5, 3 }, { 4, 3 }, { 3, 3 }, { 2, 3 } };
    std::vector<pos_t> retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -6, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 6, 3 }, { 4, 3 } }));
}

TEST_F(WolfeeMagicTest, Test3)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 3 2 2 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    std::vector<pos_t> path = { { 5, 3 }, { 4, 3 }, { 3, 3 }, { 2, 3 } };
    std::vector<pos_t> retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -6, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 5, 3 }, { 4, 3 } }));
}

TEST_F(WolfeeMagicTest, Test4)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 6 3 2 2 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    std::vector<pos_t> path = { { 6, 3 }, { 5, 3 }, { 4, 3 }, { 3, 3 }, { 2, 3 } };
    std::vector<pos_t> retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -7, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 4, 3 } }));
}

TEST_F(WolfeeMagicTest, Test5)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 6 2 5 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    std::vector<pos_t> path = { { 5, 6 }, { 5, 5 }, { 5, 4 }, { 5, 3 }, { 4, 3 }, { 3, 3 }, { 2, 3 } };
    std::vector<pos_t> retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -7, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 5, 4 }, { 5, 3 }, { 4, 3 } }));
}

TEST_F(WolfeeMagicTest, Test6)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 3 1 2 5 2 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    std::vector<pos_t> path = { { 3, 1 }, { 2, 1 }, { 1, 1 }, { 1, 2 } };
    std::vector<pos_t> retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -6, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>());
}

TEST_F(WolfeeMagicTest, Test7)
{
    std::vector<std::string> info;
    std::vector<pos_t> path;
    std::vector<pos_t> retVal;
    TickDescription tick;

    // clang-format off
    info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 3 2 2 2 0"
    };
    // clang-format on
    tick = parseTickDescription(info);
    path = { { 5, 3 }, { 4, 3 }, { 3, 3 }, { 2, 3 } };
    retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -6, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 5, 3 }, { 4, 3 } }));

    // clang-format off
    info = {
        "REQ 775 0 1",
        "VAMPIRE 1 4 3 2 1 2 0",
        "GRENADE 1 5 3 5 2"
    };
    // clang-format on
    tick = parseTickDescription(info);
    path = { { 4, 3 }, { 3, 3 }, { 2, 3 } };
    retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -5, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 4, 3 } }));
}

TEST_F(WolfeeMagicTest, Test8)
{
    std::vector<std::string> info;
    std::vector<pos_t> path;
    std::vector<pos_t> retVal;
    TickDescription tick;

    // clang-format off
    info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 6 2 5 2 0"
    };
    // clang-format on
    tick = parseTickDescription(info);
    path = { { 5, 6 }, { 5, 5 }, { 5, 4 }, { 5, 3 }, { 4, 3 }, { 3, 3 }, { 2, 3 } };
    retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -7, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 5, 4 }, { 5, 3 }, { 4, 3 } }));

    // clang-format off
    info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 4 2 5 2 0"
    };
    // clang-format on
    tick = parseTickDescription(info);
    path = { { 5, 4 }, { 5, 3 }, { 4, 3 }, { 3, 3 }, { 2, 3 } };
    retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -6, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 5, 4 }, { 5, 3 }, { 4, 3 } }));

    // clang-format off
    info = {
        "REQ 775 0 1",
        "VAMPIRE 1 5 3 2 4 2 0",
        "GRENADE 1 5 4 5 2"
    };
    // clang-format on
    tick = parseTickDescription(info);
    path = { { 5, 3 }, { 4, 3 }, { 3, 3 }, { 2, 3 } };
    retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -5, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 5, 3 }, { 4, 3 } }));

    // clang-format off
    info = {
        "REQ 775 0 1",
        "VAMPIRE 1 4 3 2 4 2 0",
        "GRENADE 1 5 4 4 2",
        "GRENADE 1 5 3 5 2"
    };
    // clang-format on
    tick = parseTickDescription(info);
    path = { { 4, 3 }, { 3, 3 }, { 2, 3 } };
    retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -4, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 4, 3 } }));
}

TEST_F(WolfeeMagicTest, Test9)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 7 5 2 5 4 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    std::vector<pos_t> path = { { 7, 5 }, { 7, 4 }, { 7, 3 }, { 7, 2 } };
    std::vector<pos_t> retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -6, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 7, 5 }, { 7, 4 } }));
    // This is a kicsit geh. (baromira ki tudnank lepni a keresztezodesbol, csak az utolso elemet nem vagyunk hajlandoak oda tenni)
}

TEST_F(WolfeeMagicTest, Test10)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 7 5 2 5 4 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    std::vector<pos_t> path = { { 7, 8 }, { 7, 7 }, { 7, 6 }, { 7, 5 }, { 7, 4 }, { 7, 3 }, { 7, 2 } };
    std::vector<pos_t> retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -8, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 7, 6 } }));
}

TEST_F(WolfeeMagicTest, Test11)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 7 5 2 5 4 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    std::vector<pos_t> path = { { 7, 8 }, { 7, 7 }, { 7, 6 }, { 7, 5 }, { 7, 4 }, { 7, 3 }, { 7, 2 } };
    std::vector<pos_t> retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -9, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 7, 6 } }));
}

TEST_F(WolfeeMagicTest, Test12)
{
    // clang-format off
    std::vector<std::string> info = {
        "REQ 775 0 1",
        "VAMPIRE 1 7 18 2 5 4 0"
    };
    // clang-format on
    const TickDescription tick = parseTickDescription(info);
    std::vector<pos_t> path = { { 7, 18 }, { 7, 17 }, { 7, 16 }, { 7, 15 }, { 7, 14 }, { 7, 13 }, { 7, 12 }, { 7, 11 }, { 7, 10 }, { 7, 9 }, { 7, 8 }, { 7, 7 },
        { 7, 6 }, { 7, 5 }, { 7, 4 }, { 7, 3 }, { 7, 2 } };
    std::vector<pos_t> retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -15, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 7, 6 } }));
}

TEST_F(WolfeeMagicTest, Test13)
{
    std::vector<std::string> info;
    std::vector<pos_t> path;
    std::vector<pos_t> retVal;
    TickDescription tick;

    // clang-format off
    info = {
        "REQ 775 0 1",
        "VAMPIRE 1 19 5 2 5 4 0"
    };
    // clang-format on
    tick = parseTickDescription(info);
    path = { { 19, 5 }, { 18, 5 }, { 17, 5 }, { 16, 5 }, { 15, 5 }, { 15, 6 }, { 15, 7 }, { 15, 8 }, { 15, 9 }, { 15, 10 }, { 15, 11 }, { 15, 12 } };
    retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -6, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 19, 5 }, { 15, 5 }, { 15, 8 } }));

    // clang-format off
    info = {
        "REQ 775 0 1",
        "VAMPIRE 1 17 5 2 4 4 0",
        "GRENADE 1 19 5 5 4",
    };
    // clang-format on
    tick = parseTickDescription(info);
    path = { { 17, 5 }, { 16, 5 }, { 15, 5 }, { 15, 6 }, { 15, 7 }, { 15, 8 }, { 15, 9 }, { 15, 10 }, { 15, 11 }, { 15, 12 } };
    retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -5, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 15, 5 }, { 15, 8 } }));

    // clang-format off
    info = {
        "REQ 775 0 1",
        "VAMPIRE 1 15 5 2 4 4 0",
        "GRENADE 1 19 5 4 4",
    };
    // clang-format on
    tick = parseTickDescription(info);
    path = { { 15, 5 }, { 15, 6 }, { 15, 7 }, { 15, 8 }, { 15, 9 }, { 15, 10 }, { 15, 11 }, { 15, 12 } };
    retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -4, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 15, 5 }, { 15, 8 } }));

    // clang-format off
    info = {
        "REQ 775 0 1",
        "VAMPIRE 1 15 7 2 4 4 0",
        "GRENADE 1 19 5 3 4",
        "GRENADE 1 15 5 5 4"
    };
    // clang-format on
    tick = parseTickDescription(info);
    path = { { 15, 7 }, { 15, 8 }, { 15, 9 }, { 15, 10 }, { 15, 11 }, { 15, 12 } };
    retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -3, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 15, 8 } }));

    // clang-format off
    info = {
        "REQ 775 0 1",
        "VAMPIRE 1 15 8 2 4 4 0",
        "GRENADE 1 19 5 2 4",
        "GRENADE 1 15 5 4 4"
    };
    // clang-format on
    tick = parseTickDescription(info);
    path = { { 15, 8 }, { 15, 9 }, { 15, 10 }, { 15, 11 }, { 15, 12 } };
    retVal = GetChainAttackBombSequenceForGaborAndKovi(path, -2, mGameDescripton, tick);
    EXPECT_EQ(retVal, std::vector<pos_t>({ { 15, 8 } }));
}

// a forbidden area-kat (mas boNbak) kivulrol kene kapni
// vagy a ticket magat.