#include "search.h"
#include "action_sequence.h"
#include "gabor_magic.h"
#include <boost/iterator/filter_iterator.hpp>
#include <cmath>
#include <iostream>
#include <optional>

bool Search::CalculateNextLevel(std::chrono::time_point<std::chrono::steady_clock> deadline)
{
    [[maybe_unused]] const int currentLevelIndex = static_cast<int>(mLevels.size());
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

            if (action.GetNumberOfSteps() > 1) {
                const auto firstStep = action.GetNthStep(0);
                const auto secondStep = action.GetNthStep(1);

                if ((firstStep == 0 && secondStep == 1) || (firstStep == 1 && secondStep == 0) || (firstStep == 2 && secondStep == 3)
                    || (firstStep == 3 && secondStep == 2)) {
                    continue;
                }

                if (action.GetNumberOfSteps() == 3) {
                    const auto thirdStep = action.GetNthStep(1);
                    if ((thirdStep == 0 && secondStep == 1) || (thirdStep == 1 && secondStep == 0) || (thirdStep == 2 && secondStep == 3)
                        || (thirdStep == 3 && secondStep == 2)) {
                        continue;
                    }
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

            if (action.GetNumberOfSteps() > 0) {
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
            }

            if (mTomatoSafePlay && std::find_if(std::cbegin(tick.mPowerUps), std::cend(tick.mPowerUps), [&nextX, &nextY](const PowerUp& powerup) {
                    return powerup.mType == PowerUp::Type::Tomato && powerup.mX == nextX && powerup.mY == nextY;
                }) != std::cend(tick.mPowerUps)) {
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
            nextLevel.emplace_back(nodeIndex, tickDescription,
                node.mPermanentScore + newPoints.at(mPlayerId) * std::pow(0.99F, static_cast<float>(currentLevelIndex)),
                node.mHeuristicScore + heuristicScore * std::pow(0.99F, static_cast<float>(currentLevelIndex)), action.GetId());
        }
    }

    if (nextLevel.empty() || nextLevel.size() < currentLevel.size()) {
        mLevels.resize(mLevels.size() - 1);
        return false;
    }

    return true;
}

Answer Search::GetBestMove()
{
    [[maybe_unused]] const auto printBranch = [&mLevels = mLevels](const TreeNode& node) {
        std::vector<ActionSequence> actions;

        const TreeNode* current = &node;
        for (size_t level = mLevels.size() - 1; level > 1; --level) {
            actions.emplace_back(current->mAction);
            current = &mLevels[level - 1][current->mParentIndex];
        }
        actions.emplace_back(current->mAction);

        std::reverse(std::begin(actions), std::end(actions));

        std::cerr << "Permanent score: " << node.mPermanentScore << " Heuristic score: " << node.mHeuristicScore << std::endl;
        for (const auto& a : actions) {
            const auto answer = a.GetAnswer();
            std::cerr << " grenade: " << answer.mPlaceGrenade << " moves: ";
            for (const auto& s : answer.mSteps) {
                std::cerr << s << ", ";
            }
            std::cerr << std::endl;
        }
    };

    const auto bestIt = std::max_element(std::cbegin(mLevels.back()), std::cend(mLevels.back()), [&](const TreeNode& x, const TreeNode& y) {
        const auto score1 = x.mPermanentScore + x.mHeuristicScore;
        const auto score2 = y.mPermanentScore + y.mHeuristicScore;

        if (std::fabs(score1 - score2) < 0.0001F) {
            // std::cerr << "score: " << score1 << std::endl;
            // printBranch(x);
            // std::cerr << std::endl;
            // printBranch(y);
            // std::cerr << "---------------" << std::endl;

            if (x.mPermanentScore != y.mPermanentScore) {
                return x.mPermanentScore < y.mPermanentScore;
            }
            const ActionSequence xAction(x.mAction);
            const ActionSequence yAction(y.mAction);

            if (xAction.GetNumberOfSteps() != yAction.GetNumberOfSteps()) {
                return xAction.GetNumberOfSteps() < yAction.GetNumberOfSteps();
            }
            return xAction.IsGrenade() < yAction.IsGrenade();
        }

        return score1 < score2;
    });

    const TreeNode* current = &*bestIt;
    for (size_t level = mLevels.size() - 1; level > 1; --level) {
        current = &mLevels[level - 1][current->mParentIndex];
    }

    std::cerr << "Permanent score: " << bestIt->mPermanentScore << " Heuristic score: " << bestIt->mHeuristicScore
              << " last level size: " << mLevels.back().size() << std::endl;
    // printBranch(*bestIt);

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
    float bombingTargetScore = 0;
    float pathTargetScore = 0;

    pos_t mypos(tickDescription.mMe.mY, tickDescription.mMe.mX);

    if (move.mPlaceGrenade && !mBombSequence.empty() && mLevels.size() <= 5 && mBombSequence.front() == mypos) {
        bombingTargetScore = 48.0F;
    }

    if (mPhase == BETWEEN_ITEMS && mLevels.size() == 2) {
        if (mPathSequence.empty() && move.mSteps.empty())
            pathTargetScore = 48.0F;
        else if (!mPathSequence.empty() && mPathSequence.back() == mypos)
            pathTargetScore = 48.0F;
    }

    std::vector<int> bombedBats(tickDescription.mAllBats.size(), 0);
    for (const auto& area : areas) {

        if (area.mArea.find(tickDescription.mMe.mX, tickDescription.mMe.mY)) {
            grenadePenalty -= 12.F / static_cast<float>(area.mTickCount);

            if (area.mTickCount == 1) {
                grenadePenalty -= 96.F;
            }
        }

        if (std::find(std::cbegin(area.mVampireIds), std::cend(area.mVampireIds), mPlayerId) == std::cend(area.mVampireIds)) {
            continue;
        }

        for (size_t batIndex = 0; batIndex < tickDescription.mAllBats.size(); ++batIndex) {
            const auto& bat = tickDescription.mAllBats[batIndex];

            if (bombedBats[batIndex] < bat.mDensity && area.mArea.find(bat.mX, bat.mY)) {
                batScore += 12.F / static_cast<float>(bat.mDensity);
                bombedBats[batIndex]++;
            }
        }
    }

    const auto closestBatIt = std::min_element(
        std::cbegin(tickDescription.mAllBats), std::cend(tickDescription.mAllBats), [&distance, &tickDescription](const BatSquad& x, const BatSquad& y) {
            return distance(x.mX, x.mY, tickDescription.mMe.mX, tickDescription.mMe.mY) < distance(y.mX, y.mY, tickDescription.mMe.mX, tickDescription.mMe.mY);
        });
    if (closestBatIt != std::cend(tickDescription.mAllBats)) {
        const auto d = distance(closestBatIt->mX, closestBatIt->mY, tickDescription.mMe.mX, tickDescription.mMe.mY);
        batScore += 0.01F / d;
    }

    const std::function<bool(const PowerUp&)> filter
        = [&simulator](const PowerUp& powerUp) { return simulator.GetReachableArea().find(powerUp.mX, powerUp.mY); };

    const auto beginIt = boost::make_filter_iterator(filter, std::cbegin(tickDescription.mPowerUps), std::cend(tickDescription.mPowerUps));
    const auto endIt = boost::make_filter_iterator(filter, std::cend(tickDescription.mPowerUps), std::cend(tickDescription.mPowerUps));

    float powerUpScore = 0;
    const auto powerUpIt = std::min_element(beginIt, endIt, [&distance, &tickDescription](const PowerUp& x, const PowerUp& y) {
        return distance(x.mX, x.mY, tickDescription.mMe.mX, tickDescription.mMe.mY) < distance(y.mX, y.mY, tickDescription.mMe.mX, tickDescription.mMe.mY);
    });
    if (powerUpIt != endIt && (!mTomatoSafePlay || powerUpIt->mType != PowerUp::Type::Tomato)) {
        const auto d = distance(powerUpIt->mX, powerUpIt->mY, tickDescription.mMe.mX, tickDescription.mMe.mY);
        powerUpScore += 48.F / (d + 1.F);
        if (powerUpIt->mRemainingTick > 0) {
            powerUpScore -= 48.F * static_cast<float>(std::distance(beginIt, endIt));
        }
    }

    const float healthPenalty = -96.F * static_cast<float>(3 - tickDescription.mMe.mHealth);

    float positionScore = 0.F;
    const auto& reachableArea = simulator.GetReachableArea();

    positionScore += 0.01F * reachableArea.find(tickDescription.mMe.mX + 1, tickDescription.mMe.mY);
    positionScore += 0.01F * reachableArea.find(tickDescription.mMe.mX - 1, tickDescription.mMe.mY);
    positionScore += 0.01F * reachableArea.find(tickDescription.mMe.mX, tickDescription.mMe.mY + 1);
    positionScore += 0.01F * reachableArea.find(tickDescription.mMe.mX, tickDescription.mMe.mY - 1);

    // const float moveScore = 0.01F * static_cast<float>(move.mSteps.size());

    (void)newPoints;
    (void)move;

    return batScore + grenadePenalty + powerUpScore + healthPenalty + positionScore + bombingTargetScore + pathTargetScore;
}