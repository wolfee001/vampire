#include "search.h"
#include "action_sequence.h"
#include <boost/iterator/filter_iterator.hpp>
#include <iostream>

bool Search::CalculateNextLevel(std::chrono::time_point<std::chrono::steady_clock> deadline)
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
                return false;
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

            const auto heuristicScore = Evaluate(tickDescription, newPoints, move);
            nextLevel.emplace_back(
                nodeIndex, tickDescription, node.mPermanentScore + newPoints.at(mPlayerId), node.mHeuristicScore + heuristicScore, action.GetId());
        }
    }

    if (nextLevel.empty()) {
        mLevels.resize(mLevels.size() - 1);
        return false;
    }

    return true;
}

Answer Search::GetBestMove()
{
    /*
    for (size_t level = mLevels.size() - 1; level >= 1; --level) {
        for (size_t i = 0; i < mLevels[level].size();) {
            const size_t parentIndex = mLevels[level][i].mParentIndex;

            for (; mLevels[level][i].mParentIndex == parentIndex && i < mLevels[level].size(); ++i) {
                mLevels[level - 1][parentIndex].mPermanentScore += mLevels[level][i].mPermanentScore + mLevels[level][i].mHeuristicScore;
            }
            mLevels[level - 1][parentIndex].mPermanentScore += mLevels[level - 1][parentIndex].mHeuristicScore;
        }
    }
*/
    const auto bestIt = std::max_element(std::cbegin(mLevels.back()), std::cend(mLevels.back()), [](const TreeNode& x, const TreeNode& y) {
        const auto score1 = x.mPermanentScore + x.mHeuristicScore;
        const auto score2 = y.mPermanentScore + y.mHeuristicScore;

        if (score1 == score2) {
            if (x.mPermanentScore != y.mPermanentScore) {
                return x.mPermanentScore != y.mPermanentScore;
            } else {
                return ActionSequence(x.mAction).GetNumberOfSteps() < ActionSequence(y.mAction).GetNumberOfSteps();
            }
        }

        return score1 < score2;
    });

    const TreeNode* current = &*bestIt;
    for (size_t level = mLevels.size() - 1; level > 1; --level) {
        current = &mLevels[level - 1][current->mParentIndex];
    }

    std::cerr << "Permanent score: " << bestIt->mPermanentScore << " Heuristic score: " << bestIt->mHeuristicScore << std::endl;

    /*
        for (auto it = std::cbegin(mLevels.back()); it != std::cend(mLevels.back()); ++it) {
            std::cerr << (it->mPermanentScore + it->mHeuristicScore);
            if (it == bestIt) {
                std::cerr << "*";
            }
            std::cerr << ", ";
        }
        std::cerr << std::endl;
    */
    return ActionSequence(current->mAction).GetAnswer();
}

float Search::Evaluate(const TickDescription& tickDescription, const Simulator::NewPoints& newPoints, const Answer& move) const
{
    const auto distance2 = [](int x, int y) -> float { return static_cast<float>(std::max(x, y) - std::min(x, y)); };
    const auto distance = [&distance2](int x1, int y1, int x2, int y2) -> float { return distance2(x1, x2) + distance2(y1, y2); };

    Simulator simulator(mGameDescription);
    simulator.SetState(tickDescription);
    const auto areas = simulator.GetBlowAreas();

    float batScore = 0;
    float grenadePenalty = 0;

    for (const auto& area : areas) {
        if (area.mArea.find(tickDescription.mMe.mX, tickDescription.mMe.mY)) {
            grenadePenalty -= 48.F / static_cast<float>(area.mTickCount);
        }

        if (std::find(std::cbegin(area.mVampireIds), std::cend(area.mVampireIds), mPlayerId) == std::cend(area.mVampireIds)) {
            continue;
        }

        for (const auto& bat : tickDescription.mAllBats) {
            if (area.mArea.find(bat.mX, bat.mY)) {
                batScore += 12.F / static_cast<float>(bat.mDensity);
            }
        }
    }

    const std::function<bool(const PowerUp&)> filter
        = [&simulator](const PowerUp& powerUp) { return !simulator.GetReachableArea().find(powerUp.mX, powerUp.mY); };

    const auto beginIt = boost::make_filter_iterator(filter, std::cbegin(tickDescription.mPowerUps), std::cend(tickDescription.mPowerUps));
    const auto endIt = boost::make_filter_iterator(filter, std::cend(tickDescription.mPowerUps), std::cend(tickDescription.mPowerUps));

    float powerUpScore = 0;
    const auto powerUpIt = std::min_element(beginIt, endIt, [&distance, &tickDescription](const PowerUp& x, const PowerUp& y) {
        return distance(x.mX, x.mY, tickDescription.mMe.mX, tickDescription.mMe.mY) < distance(y.mX, y.mY, tickDescription.mMe.mX, tickDescription.mMe.mY);
    });
    if (powerUpIt != endIt) {
        const auto d = distance(powerUpIt->mX, powerUpIt->mY, tickDescription.mMe.mX, tickDescription.mMe.mY);
        powerUpScore += 48.F / (d + 1.F);
    }

    (void)move;
    (void)newPoints;

    return batScore + (3.F - static_cast<float>(tickDescription.mMe.mHealth)) * grenadePenalty + powerUpScore + 0.01F * static_cast<float>(move.mSteps.size());
}