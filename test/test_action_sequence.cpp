#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unordered_map>

#include "../action_sequence.h"

bool operator==(const Answer& x, const Answer& y)
{
    return x.mPlaceGrenade == y.mPlaceGrenade && x.mSteps == y.mSteps;
}

TEST(ActionSequence, EmptyWithoutBomb)
{
    Answer answer;
    answer.mPlaceGrenade = false;

    const ActionSequence as(answer);
    EXPECT_EQ(as.GetAnswer(), answer);
    EXPECT_EQ(as.GetId(), 0);
}

TEST(ActionSequence, EmptyWithBomb)
{
    Answer answer;
    answer.mPlaceGrenade = true;

    const ActionSequence as(answer);
    EXPECT_EQ(as.GetAnswer(), answer);
    EXPECT_EQ(as.GetId(), 1);
}

TEST(ActionSequence, UpWithoutBomb)
{
    Answer answer;
    answer.mPlaceGrenade = false;
    answer.mSteps = { 'U' };

    const ActionSequence as(answer);
    EXPECT_EQ(as.GetAnswer(), answer);
    EXPECT_EQ(as.GetId(), 2);
}

TEST(ActionSequence, UpWithBomb)
{
    Answer answer;
    answer.mPlaceGrenade = true;
    answer.mSteps = { 'U' };

    const ActionSequence as(answer);
    EXPECT_EQ(as.GetAnswer(), answer);
    EXPECT_EQ(as.GetId(), 3);
}

TEST(ActionSequence, UpLeftRightWithBomb)
{
    Answer answer;
    answer.mPlaceGrenade = true;
    answer.mSteps = { 'U', 'L', 'R' };

    const ActionSequence as(answer);
    EXPECT_EQ(as.GetAnswer(), answer);
}

TEST(ActionSequence, AllFromAnswer)
{
    Answer answer;

    std::vector<bool> ids(170, false);

    const auto test = [&answer, &ids]() {
        const ActionSequence as(answer);
        EXPECT_EQ(as.GetAnswer(), answer);
        EXPECT_LT(as.GetId(), ids.size());
        EXPECT_FALSE(ids[as.GetId()]);
        ids[as.GetId()] = true;
    };

    answer.mSteps = {};
    answer.mPlaceGrenade = true;
    test();
    answer.mPlaceGrenade = false;
    test();

    for (size_t b = 0; b < 2; ++b) {
        answer.mPlaceGrenade = b % 2;

        for (const char c1 : { 'U', 'D', 'L', 'R' }) {
            answer.mSteps = { c1 };
            test();

            for (const char c2 : { 'U', 'D', 'L', 'R' }) {
                answer.mSteps = { c1, c2 };
                test();

                for (const char c3 : { 'U', 'D', 'L', 'R' }) {
                    answer.mSteps = { c1, c2, c3 };
                    test();
                }
            }
        }
    }

    EXPECT_TRUE(std::all_of(std::cbegin(ids), std::cend(ids), [](bool v) { return v; }));
}
