#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unordered_map>

#include "../action_sequence.h"

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
    EXPECT_EQ(as.GetId(), 1 << 8);
}

TEST(ActionSequence, UpWithBomb)
{
    Answer answer;
    answer.mPlaceGrenade = true;
    answer.mSteps = { 'U' };

    const ActionSequence as(answer);
    EXPECT_EQ(as.GetAnswer(), answer);
    EXPECT_EQ(as.GetId(), (1 << 8) | 1);
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

    const auto test = [&answer]() {
        const ActionSequence as(answer);
        EXPECT_EQ(as.GetAnswer(), answer);

        EXPECT_EQ(answer.mSteps.size(), as.GetNumberOfSteps());
        EXPECT_EQ(answer.mPlaceGrenade, as.IsGrenade());

        for (size_t i = 0; i < answer.mSteps.size(); ++i) {
            switch (answer.mSteps[i]) {
            case 'U':
                EXPECT_EQ(as.GetNthStep(static_cast<int>(i)), 0);
                break;
            case 'D':
                EXPECT_EQ(as.GetNthStep(static_cast<int>(i)), 1);
                break;
            case 'L':
                EXPECT_EQ(as.GetNthStep(static_cast<int>(i)), 2);
                break;
            case 'R':
                EXPECT_EQ(as.GetNthStep(static_cast<int>(i)), 3);
                break;
            default:
                throw std::runtime_error("Invalid step");
            }
        }
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
}

TEST(ActionSequence, GetNthStep)
{
    Answer answer;

    {
        answer.mSteps.push_back('U');
        const ActionSequence action(answer);
        EXPECT_EQ(action.GetAnswer().mSteps[0], 'U');
        EXPECT_EQ(action.GetNthStep(0), 0);
    }

    {
        answer.mSteps = { 'U', 'U', 'D' };
        const ActionSequence action(answer);
        EXPECT_EQ(action.GetAnswer().mSteps[2], 'D');
        EXPECT_EQ(action.GetNthStep(2), 1);
    }
}