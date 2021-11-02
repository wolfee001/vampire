#include "search.h"
#include "action_sequence.h"
#include <iostream>

void Search::CalculateNextLevel(std::chrono::time_point<std::chrono::steady_clock> deadline)
{
    const int currentLevelIndex = static_cast<int>(mLevels.size());
    mLevels.resize(mLevels.size() + 1);

    const auto& currentLevel = mLevels[mLevels.size() - 2];
    auto& nextLevel = mLevels.back();
    nextLevel.reserve(currentLevel.size() * 3);

    Simulator simulator(mGameDescription);

    for (uint32_t nodeIndex = 0; nodeIndex < currentLevel.size(); ++nodeIndex) {
        const TreeNode& node = currentLevel[nodeIndex];

        TickDescription tick = node.mTickDescription;
        tick.mEnemyVampires.clear();

        simulator.SetState(tick);

        for (ActionSequence::ActionSequence_t i = 0; i <= ActionSequence::MaxSequenceId; ++i) {
            if (std::chrono::steady_clock::now() > deadline) {
                mLevels.resize(mLevels.size() - 1);
                return;
            }

            const ActionSequence action(i);
            if (action.GetNumberOfSteps() == 3 && node.mTickDescription.mMe.mRunningShoesTick == 0) {
                continue;
            }
            if (action.IsGrenade() && node.mTickDescription.mMe.mPlacableGrenades == 0) {
                continue;
            }

            if (action.GetNumberOfSteps() == 2) {
                const auto firstStep = action.GetNthStep(0);
                const auto secondStep = action.GetNthStep(1);

                // visszamozgás
                if ((firstStep == 0 && secondStep == 1) || (firstStep == 1 && secondStep == 0) || (firstStep == 2 && secondStep == 3)
                    || (firstStep == 3 && secondStep == 2)) {
                    continue;
                }
            }

            if (action.IsGrenade() && action.GetNumberOfSteps() < 2) {
                continue;
            }

            int nextX = node.mTickDescription.mMe.mX;
            int nextY = node.mTickDescription.mMe.mY;
            for (int stepIndex = 0; stepIndex < action.GetNumberOfSteps(); ++stepIndex) {
                const auto stepId = action.GetNthStep(stepIndex);
                switch (stepId) {
                case 0:
                    --nextY;
                    break;
                case 1:
                    ++nextY;
                    break;
                case 2:
                    --nextX;
                    break;
                case 3:
                    ++nextX;
                    break;
                }
            }

            const TreeNode* searchNode = &node;
            bool sameAsPreviousStatus = false;
            for (int level = currentLevelIndex - 2; level >= 1; --level) {
                searchNode = &(mLevels[static_cast<size_t>(level)][searchNode->mParentIndex]);
                if (ActionSequence(searchNode->mAction).IsGrenade()) {
                    break;
                }

                if (searchNode->mTickDescription.mMe.mX == nextX && searchNode->mTickDescription.mMe.mY == nextY) {
                    sameAsPreviousStatus = true;
                    break;
                }
            }
            if (sameAsPreviousStatus) {
                continue;
            }

            const Answer move = action.GetAnswer();
            if (!simulator.IsValidMove(mPlayerId, move)) {
                continue;
            }

            simulator.SetVampireMove(mPlayerId, move);
            const auto [tickDescription, newPoints] = simulator.Tick();
            simulator.SetState(tick);

            const auto score = Evaluate(tickDescription, newPoints, move);
            nextLevel.emplace_back(nodeIndex, tickDescription, score, action.GetId());
        }
    }
}

Answer Search::GetBestMove()
{
    for (size_t level = mLevels.size() - 1; level >= 1; --level) {

        for (size_t i = 0; i < mLevels[level].size();) {
            const size_t parentIndex = mLevels[level][i].mParentIndex;

            mLevels[level - 1][parentIndex].mScore = 0;

            for (; mLevels[level][i].mParentIndex == parentIndex && i < mLevels[level].size(); ++i) {
                mLevels[level - 1][parentIndex].mScore += mLevels[level][i].mScore;

                mLevels[level - 1][parentIndex].mNumberOfChildren += mLevels[level][i].mNumberOfChildren + 1;
            }
        }
    }

    const auto bestIt = std::max_element(std::cbegin(mLevels[1]), std::cend(mLevels[1]), [](const TreeNode& x, const TreeNode& y) {
        const auto score1 = x.mScore / static_cast<float>(x.mNumberOfChildren);
        const auto score2 = y.mScore / static_cast<float>(y.mNumberOfChildren);

        if (std::fabs(score1 - score2) < 0.1F) {
            return ActionSequence(x.mAction).GetNumberOfSteps() < ActionSequence(y.mAction).GetNumberOfSteps();
        } else {
            return score1 < score2;
        }
    });

    for (auto it = std::cbegin(mLevels[1]); it != std::cend(mLevels[1]); ++it) {
        if (it == bestIt) {
            std::cerr << (it->mScore / static_cast<float>(it->mNumberOfChildren)) << "*, ";
        } else {
            std::cerr << (it->mScore / static_cast<float>(it->mNumberOfChildren)) << ", ";
        }
    }

    return ActionSequence(bestIt->mAction).GetAnswer();
}

float Search::Evaluate(const TickDescription& tickDescription, const Simulator::NewPoints& newPoints, const Answer& move) const
{
    const auto distance2 = [](int x, int y) -> float { return static_cast<float>(std::max(x, y) - std::min(x, y)); };

    const auto distance = [&distance2](int x1, int y1, int x2, int y2) -> float { return distance2(x1, x2) + distance2(y1, y2); };

    float batScore = 0;

    const auto myGrenade = std::find_if(std::cbegin(tickDescription.mGrenades), std::cend(tickDescription.mGrenades),
        [&tickDescription](const Grenade& g) { return g.mId == tickDescription.mMe.mId; });
    if (myGrenade != std::cend(tickDescription.mGrenades)) {
        for (const auto& bat : tickDescription.mAllBats) {
            if (distance2(bat.mX, myGrenade->mX) <= static_cast<float>(myGrenade->mRange)
                && distance2(bat.mY, myGrenade->mY) <= static_cast<float>(myGrenade->mRange)) {
                batScore += 12 / static_cast<float>(bat.mDensity);
            }
        }
    }

    const auto batIt = std::min_element(
        std::cbegin(tickDescription.mAllBats), std::cend(tickDescription.mAllBats), [&tickDescription, &distance](const BatSquad& x, const BatSquad& y) {
            return distance(tickDescription.mMe.mX, tickDescription.mMe.mY, x.mX, x.mY) < distance(tickDescription.mMe.mX, tickDescription.mMe.mY, y.mX, y.mY);
        });

    if (batIt != std::cend(tickDescription.mAllBats)) {
        batScore += 12.F / distance(tickDescription.mMe.mX, tickDescription.mMe.mY, batIt->mX, batIt->mY);
    }

    float grenadePenalty = 0;
    const auto closeGrenadeIt = std::min_element(
        std::cbegin(tickDescription.mGrenades), std::cend(tickDescription.mGrenades), [&tickDescription, &distance](const Grenade& x, const Grenade& y) {
            return distance(tickDescription.mMe.mX, tickDescription.mMe.mY, x.mX, x.mY) < distance(tickDescription.mMe.mX, tickDescription.mMe.mY, y.mX, y.mY);
        });
    if (closeGrenadeIt != std::cend(tickDescription.mGrenades)) {
        if (distance2(tickDescription.mMe.mX, closeGrenadeIt->mX) < static_cast<float>(closeGrenadeIt->mRange + 2)
            || distance2(tickDescription.mMe.mY, closeGrenadeIt->mY) < static_cast<float>(closeGrenadeIt->mRange + 2)) {
            grenadePenalty -= 48.F / static_cast<float>(closeGrenadeIt->mTick);
        }
    }

    return newPoints.at(mPlayerId) + 0.001F * static_cast<float>(move.mSteps.size()) + batScore + grenadePenalty;
}