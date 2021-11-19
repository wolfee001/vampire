#include "search.h"
#include "action_sequence.h"
#include "check.h"
#include "gabor_magic.h"
#include <limits>

#ifdef _WIN32
#pragma warning(push, 0)
#endif

// #include <boost/container/flat_map.hpp>
// #include <boost/iterator/filter_iterator.hpp>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include <cmath>
#include <iostream>
#include <optional>

Search::Search(const TickDescription& tickDescription, const GameDescription& gameDescription, int playerId)
    : mGameDescription(gameDescription)
    , mPlayerId(playerId)
{
    const auto heuristicScore = Evaluate(tickDescription, Simulator::NewPoints { { mPlayerId, 0.F } }, {}, 0);
    mLevels.reserve(10);

    const auto grenadeIt
        = std::find_if(std::cbegin(tickDescription.mGrenades), std::cend(tickDescription.mGrenades), [&playerId](const auto& x) { return x.mId == playerId; });

    const ActionSequence action(Answer { grenadeIt != std::cend(tickDescription.mGrenades), {} });

    auto& firstNode = mLevels.emplace_back().emplace_back(std::numeric_limits<uint32_t>::max(), tickDescription, 0.F, heuristicScore, action.GetId());
    auto& firstTick = firstNode.mTickDescription;

    firstTick.mPowerUps.erase(std::remove_if(std::begin(firstTick.mPowerUps), std::end(firstTick.mPowerUps),
                                  [&firstTick](const PowerUp& powerUp) {
                                      return powerUp.mRemainingTick >= -1
                                          && std::find_if(std::cbegin(firstTick.mEnemyVampires), std::cend(firstTick.mEnemyVampires),
                                                 [&powerUp](const Vampire& vampire) {
                                                     return vampire.mX == powerUp.mX && vampire.mY == powerUp.mY && vampire.mRestCount >= powerUp.mDefensTime;
                                                 })
                                          != std::cend(firstTick.mEnemyVampires);
                                  }),
        std::cend(firstTick.mPowerUps));

    mMyOriginalPos = pos_t(tickDescription.mMe.mY, tickDescription.mMe.mX);

    mTomatoSafePlay = false;
}

bool Search::CalculateNextLevel(std::chrono::time_point<std::chrono::steady_clock> deadline)
{
    [[maybe_unused]] const auto currentLevelIndex = mLevels.size();
    mLevels.resize(mLevels.size() + 1);

    const auto& currentLevel = mLevels[mLevels.size() - 2];
    auto& nextLevel = mLevels.back();
    nextLevel.reserve(currentLevel.size() * 3);

    const auto FindGrenadeThrowDistance = [&mGameDescription = mGameDescription](const TickDescription& tick, Answer move) -> uint8_t {
        Simulator simulator(mGameDescription);
        simulator.SetState(tick);

        CHECK(move.mThrow, "missing throw");

        for (int length = mGameDescription.mGrenadeRadius + 1; length >= 0; --length) {
            move.mThrow->mDistance = length;

            if (simulator.GetThrowPosition(std::make_pair(tick.mMe.mX, tick.mMe.mY), *move.mThrow)) {
                return static_cast<uint8_t>(length);
            }
        }

        return 0;
    };

    Simulator simulator(mGameDescription);

    for (uint32_t nodeIndex = 0; nodeIndex < currentLevel.size(); ++nodeIndex) {
        const TreeNode& node = currentLevel[nodeIndex];

        TickDescription tick = node.mTickDescription;
        tick.mEnemyVampires.clear();

        simulator.SetState(tick);

        ActionSequence action(0);

        do {
            Answer move = action.GetAnswer();

            if (action.IsGrenade() && action.IsThrow()) {
                continue;
            }

            if (action.IsThrow()) {
                int grenadeX = node.mTickDescription.mMe.mX;
                int grenadeY = node.mTickDescription.mMe.mY;

                switch (action.GetThrowDirection()) {
                case Throw::Direction::Up:
                    --grenadeY;
                    break;
                case Throw::Direction::Down:
                    ++grenadeY;
                    break;
                case Throw::Direction::Left:
                    --grenadeX;
                    break;
                case Throw::Direction::Right:
                    ++grenadeX;
                    break;

                case Throw::Direction::XUp:
                case Throw::Direction::XDown:
                case Throw::Direction::XLeft:
                case Throw::Direction::XRight:
                    break;
                }

                const auto gIt = std::find_if(std::cbegin(node.mTickDescription.mGrenades), std::cend(node.mTickDescription.mGrenades),
                    [&grenadeX, &grenadeY](const Grenade& g) { return g.mX == grenadeX && g.mY == grenadeY; });

                if (gIt == std::cend(node.mTickDescription.mGrenades)) {
                    continue;
                }

                move.mThrow->mDistance = FindGrenadeThrowDistance(node.mTickDescription, move);
                if (move.mThrow->mDistance == 0) {
                    continue;
                }
            }

            const auto now = std::chrono::steady_clock::now();
            if (now > deadline) {
                mLevels.resize(mLevels.size() - 1);
                return false;
            }

            if (mThrow && action.IsGrenade()) {
                continue;
            }

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
                    const auto thirdStep = action.GetNthStep(2);
                    if ((thirdStep == 0 && secondStep == 1) || (thirdStep == 1 && secondStep == 0) || (thirdStep == 2 && secondStep == 3)
                        || (thirdStep == 3 && secondStep == 2)) {
                        continue;
                    }
                }
            }

            if (action.IsGrenade() && action.GetNumberOfSteps() == 1) {
                continue;
            }
            /*
                        if (action.IsGrenade() && action.GetNumberOfSteps() < 2) {
                            if (mLevels.size() > 2 || !mPreferGrenade || action.GetNumberOfSteps() == 1) {
                                continue;
                            }
                        }
            */
            if (mLevels.size() == 2 && (mAvoids & 16) && action.GetNumberOfSteps() == 0) {
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
                for (int level = static_cast<int>(currentLevelIndex) - 2; level >= 1; --level) {
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

            if (!simulator.IsValidMove(mPlayerId, action)) {
                continue;
            }

            if (mLevels.size() == 2 && mAvoids && action.GetNumberOfSteps() > 0) {
                if ((mAvoids & 1) && move.mSteps[0] == 'U') {
                    continue;
                }
                if ((mAvoids & 2) && move.mSteps[0] == 'R') {
                    continue;
                }
                if ((mAvoids & 4) && move.mSteps[0] == 'D') {
                    continue;
                }
                if ((mAvoids & 8) && move.mSteps[0] == 'L') {
                    continue;
                }
            }

            if (currentLevelIndex == 1) {
                if (mThrow) {
                    move.mThrow = mThrow;
                }
            }

            simulator.SetVampireMove(mPlayerId, move);
            const auto [tickDescription, newPoints] = simulator.Tick();
            simulator.SetState(tick);

            const auto heuristicScore = Evaluate(tickDescription, newPoints, move, currentLevelIndex);
            auto& nextNode = nextLevel.emplace_back(nodeIndex, tickDescription,
                node.mPermanentScore + newPoints.at(mPlayerId) * std::pow(0.99F, static_cast<float>(currentLevelIndex)),
                node.mHeuristicScore + heuristicScore * std::pow(0.99F, static_cast<float>(currentLevelIndex)), action.GetId());

            nextNode.mThrowLength = static_cast<uint8_t>(move.mThrow ? move.mThrow->mDistance : 0);
        } while (action.GetNextId());
    }

    if (nextLevel.empty() || nextLevel.size() < currentLevel.size()) {
        std::cerr << "next level is empty or shrinking!" << std::endl;
        mLevels.resize(mLevels.size() - 1);
        return false;
    }

    return true;
}

Answer Search::GetBestMove()
{
    [[maybe_unused]] const auto printBranch = [&mLevels = mLevels, this](const TreeNode& node, const bool debug) {
        std::vector<const TreeNode*> branch;

        const TreeNode* current = &node;
        for (size_t level = mLevels.size() - 1; level > 1; --level) {
            branch.emplace_back(current);
            current = &mLevels[level - 1][current->mParentIndex];
        }
        branch.emplace_back(current);

        std::reverse(std::begin(branch), std::end(branch));

        std::cerr << "Permanent score: " << node.mPermanentScore << " Heuristic score: " << node.mHeuristicScore << std::endl;
        size_t level = 1;
        for (const auto& currentNode : branch) {
            const ActionSequence action(currentNode->mAction);
            std::cerr << "Level " << level << " ";
            action.Print();

            if (debug) {
                Evaluate(currentNode->mTickDescription, Simulator::NewPoints {}, action.GetAnswer(), level, true);
            }
            ++level;
        }
    };

    if (mPhase != ITEM) {
        CalculateMoveRestrictions();
    }

    const auto bestIt = std::max_element(std::cbegin(mLevels.back()), std::cend(mLevels.back()), [&](const TreeNode& x, const TreeNode& y) {
        const auto score1 = x.mPermanentScore + x.mHeuristicScore + x.mRestrictionScore;
        const auto score2 = y.mPermanentScore + y.mHeuristicScore + y.mRestrictionScore;

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
    printBranch(*bestIt, false);
    /*
        for (size_t i = 0; i < mLevels.back().size(); ++i) {
            std::cerr << i << std::endl;
            printBranch(mLevels.back()[i]);
        }
    */
    Answer answer = ActionSequence(current->mAction).GetAnswer();
    if (mThrow) {
        answer.mThrow = mThrow;
    } else {
        if (answer.mThrow) {
            answer.mThrow->mDistance = current->mThrowLength;
        }
    }
    return answer;
}

float Search::Evaluate(const TickDescription& tickDescription, const Simulator::NewPoints& newPoints, const Answer& move, const size_t level,
    const bool printScores /*= false*/) const
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

    const pos_t mypos(tickDescription.mMe.mY, tickDescription.mMe.mX);

    if (mPhase == PHASE1 && !mBombSequence.empty()) {
        for (size_t bombIndex = 0; bombIndex < mBombSequence.size(); ++bombIndex) {
            const auto& bombingPlace = mBombSequence[bombIndex];

            const auto gIt = std::find_if(std::cbegin(tickDescription.mGrenades), std::cend(tickDescription.mGrenades),
                [&bombingPlace](const Grenade& grenade) { return bombingPlace.x == grenade.mX && bombingPlace.y == grenade.mY; });

            if (gIt != std::cend(tickDescription.mGrenades)) {
                // reward earch covered bombing place, prioritize the first one
                bombingTargetScore += 12.F * (bombIndex == 0 ? 1.F : 0.2F);
            }
        }
    }

    if (mPhase == WOLFEE && !mBombSequence.empty()) {
        for (size_t bombIndex = 0; bombIndex < mBombSequence.size(); ++bombIndex) {
            const auto& bombingPlace = mBombSequence[bombIndex];

            const auto gIt = std::find_if(std::cbegin(tickDescription.mGrenades), std::cend(tickDescription.mGrenades),
                [&bombingPlace](const Grenade& grenade) { return bombingPlace.x == grenade.mX && bombingPlace.y == grenade.mY; });

            if (gIt != std::cend(tickDescription.mGrenades)) {
                // reward earch covered bombing place, prioritize the first one
                bombingTargetScore += 96.F * (bombIndex == 0 ? 1.F : 0.2F);
            }
        }
    }

    if ((mPhase == BETWEEN_ITEMS || mPhase == CHARGE) && level == 1) {
        if (mPathSequence.empty() && move.mSteps.empty()) {
            pathTargetScore = 3.0F;
        } else if (!mPathSequence.empty() && mPathSequence.back() == mypos) {
            pathTargetScore = 3.0F;
        }
    }

    if (mPreferGrenade) {
        if (level == 1 && move.mPlaceGrenade) {
            bombingTargetScore += 96.F;
        } else if (level == 2 && mPreferGrenade == 2 && move.mPlaceGrenade) {
            bombingTargetScore += 48.F;
        } else if (level >= 2) {
            auto p = mMyOriginalPos;
            const auto gIt = std::find_if(std::cbegin(tickDescription.mGrenades), std::cend(tickDescription.mGrenades),
                [&p](const Grenade& grenade) { return p.x == grenade.mX && p.y == grenade.mY; });
            if (gIt != std::cend(tickDescription.mGrenades)) {
                bombingTargetScore += 24.F;
            }
        }
    }

    if ((mPhase == ITEM || mPhase == WOLFEE) && !mPathSequence.empty()) {
        const auto powerUpIt = std::find_if(std::cbegin(tickDescription.mPowerUps), std::cend(tickDescription.mPowerUps),
            [&mPathSequence = mPathSequence](const PowerUp& powerup) { return powerup.mX == mPathSequence.back().x && powerup.mY == mPathSequence.back().y; });
        if (powerUpIt != std::cend(tickDescription.mPowerUps) && (powerUpIt->mType != PowerUp::Type::Tomato || !mTomatoSafePlay)) {
            const auto pathIt = std::find(std::cbegin(mPathSequence), std::cend(mPathSequence), mypos);
            if (pathIt != std::cend(mPathSequence)) {
                pathTargetScore += (1.1F * static_cast<float>(move.mSteps.size())) * (12.F / static_cast<float>(mPathSequence.size()))
                    * static_cast<float>(std::distance(pathIt, std::cend(mPathSequence)));
            }
        }
    }

    if (level == 1) {
        for (const auto& grenade : tickDescription.mGrenades) {
            if (grenade.mId != tickDescription.mMe.mId) {
                continue;
            }

            const auto area = simulator.GetBlowArea(grenade);

            for (const auto& vampire : mLevels.front().front().mTickDescription.mEnemyVampires) {
                if (area.find(vampire.mX, vampire.mY)) {
                    bombingTargetScore += 12.F;
                }
            }
        }
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

        bool hasBatTarget = false;

        for (size_t batIndex = 0; batIndex < tickDescription.mAllBats.size(); ++batIndex) {
            const auto& bat = tickDescription.mAllBats[batIndex];

            if (bombedBats[batIndex] < bat.mDensity && area.mArea.find(bat.mX, bat.mY)) {
                batScore += 12.F;
                bombedBats[batIndex]++;

                if (mPhase == ITEM) {
                    const pos_t batPosition(bat.mY, bat.mX);
                    const auto pathIt = std::find(std::cbegin(mPathSequence), std::cend(mPathSequence), batPosition);
                    if (pathIt != std::cend(mPathSequence)) {
                        batScore += 12.F;
                    }
                }
                hasBatTarget = true;
            }
        }

        if (!hasBatTarget) {
            batScore -= 12.F;
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

    const auto& reachableArea = simulator.GetReachableArea();

    float powerUpScore = 0;
    const PowerUp* powerUpPtr = nullptr;

    if (mPhase == ITEM && !mPathSequence.empty()) {
        const auto powerUpIt = std::find_if(std::cbegin(tickDescription.mPowerUps), std::cend(tickDescription.mPowerUps),
            [&mPathSequence = mPathSequence](const PowerUp& powerup) { return powerup.mX == mPathSequence.back().x && powerup.mY == mPathSequence.back().y; });

        if (powerUpIt != std::cend(tickDescription.mPowerUps)) {
            powerUpPtr = &(*powerUpIt);
        }

    } else {
        const std::function<bool(const PowerUp&)> isValidPowerUp = [&reachableArea, mMyOriginalPos = mMyOriginalPos](const PowerUp& powerUp) {
            // ha nincs bat vagy gránát alatta, kivéve ha már rajta vagyok, akkor lehet alattam gránát
            return reachableArea.find(powerUp.mX, powerUp.mY) || (powerUp.mX == mMyOriginalPos.x && powerUp.mY == mMyOriginalPos.y);
        };

        /*
        const auto beginIt = boost::make_filter_iterator(isValidPowerUp, std::cbegin(tickDescription.mPowerUps), std::cend(tickDescription.mPowerUps));
        const auto endIt = boost::make_filter_iterator(isValidPowerUp, std::cend(tickDescription.mPowerUps), std::cend(tickDescription.mPowerUps));

        const auto powerUpIt = std::min_element(beginIt, endIt, [&distance, &tickDescription](const PowerUp& x, const PowerUp& y) {
                    return distance(x.mX, x.mY, tickDescription.mMe.mX, tickDescription.mMe.mY) < distance(y.mX, y.mY, tickDescription.mMe.mX,
           tickDescription.mMe.mY);
        });
        */

        float minDistance = std::numeric_limits<float>::max();
        for (const auto& powerUp : tickDescription.mPowerUps) {
            if (!isValidPowerUp(powerUp)) {
                continue;
            }
            const auto d = distance(powerUp.mX, powerUp.mY, tickDescription.mMe.mX, tickDescription.mMe.mY);
            if (d < minDistance) {
                minDistance = d;
                powerUpPtr = &powerUp;
            }
        }

        // if (powerUpIt != endIt) {
        //    powerUpPtr = &(*powerUpIt);
        //}
    }

    if (powerUpPtr != nullptr && (!mTomatoSafePlay || powerUpPtr->mType != PowerUp::Type::Tomato)) {
        const auto d = distance(powerUpPtr->mX, powerUpPtr->mY, tickDescription.mMe.mX, tickDescription.mMe.mY);
        if (powerUpPtr->mRemainingTick > 0 && tickDescription.mMe.mRestCount >= powerUpPtr->mDefensTime) {
            powerUpScore -= 48.F;
        } else {
            powerUpScore += 48.F / (1.F + d);
        }
    }

    const float healthPenalty = -96.F * static_cast<float>(3 - tickDescription.mMe.mHealth);

    float positionScore = 0.F;
    positionScore += 0.01F * reachableArea.find(tickDescription.mMe.mX + 1, tickDescription.mMe.mY);
    positionScore += 0.01F * reachableArea.find(tickDescription.mMe.mX - 1, tickDescription.mMe.mY);
    positionScore += 0.01F * reachableArea.find(tickDescription.mMe.mX, tickDescription.mMe.mY + 1);
    positionScore += 0.01F * reachableArea.find(tickDescription.mMe.mX, tickDescription.mMe.mY - 1);

    // const float moveScore = 0.01F * static_cast<float>(move.mSteps.size());

    (void)newPoints;
    (void)move;

    if (printScores) {
        std::cerr << "Evaluation scores: " << std::endl;
        std::cerr << "     - batScore: " << batScore << std::endl;
        std::cerr << "     - grenadePenalty: " << grenadePenalty << std::endl;
        std::cerr << "     - powerUpScore: " << powerUpScore << std::endl;
        std::cerr << "     - healthPenalty: " << healthPenalty << std::endl;
        std::cerr << "     - positionScore: " << positionScore << std::endl;
        std::cerr << "     - bombingTargetScore: " << bombingTargetScore << std::endl;
        std::cerr << "     - pathTargetScore: " << pathTargetScore << std::endl;
        std::cerr << std::endl;
    }

    return batScore + grenadePenalty + powerUpScore + healthPenalty + positionScore + bombingTargetScore + pathTargetScore;
}

void Search::CalculateMoveRestrictions([[maybe_unused]] const bool defense /* = true */)
{
    if (mLevels.size() < 2) {
        std::cerr << "Move restriction heuristic: too few levels are calculated" << std::endl;
        return;
    }

    auto& rootTick = mLevels.front().front().mTickDescription;
    if (rootTick.mEnemyVampires.empty()) {
        std::cerr << "Move restriction heuristic: missing enemy vampires" << std::endl;
        return;
    }

    const auto SimulateMove = [](int x, int y, const ActionSequence& move) {
        const auto steps = move.GetNumberOfSteps();

        for (int n = 0; n < steps; ++n) {
            const auto d = move.GetNthStep(n);
            switch (d) {
            case 0:
                y--;
                break;
            case 1:
                y++;
                break;
            case 2:
                x--;
                break;
            case 3:
                x++;
                break;
            default:
                throw std::runtime_error("Invalid step");
            }
        }
        return pos_t(y, x);
    };

    const auto GetMovesWithoutBombing = [&mGameDescription = mGameDescription](const TickDescription& tick, const int playerId) -> std::vector<ActionSequence> {
        Simulator simulator(mGameDescription);
        simulator.SetState(tick);

        std::vector<ActionSequence> moves;

        ActionSequence move(0);
        do {
            if (move.IsGrenade() || move.IsThrow()) {
                continue;
            }

            if (move.GetNumberOfSteps() > 1) {
                const auto firstStep = move.GetNthStep(0);
                const auto secondStep = move.GetNthStep(1);

                if ((firstStep == 0 && secondStep == 1) || (firstStep == 1 && secondStep == 0) || (firstStep == 2 && secondStep == 3)
                    || (firstStep == 3 && secondStep == 2)) {
                    continue;
                }

                if (move.GetNumberOfSteps() == 3) {
                    const auto thirdStep = move.GetNthStep(2);
                    if ((thirdStep == 0 && secondStep == 1) || (thirdStep == 1 && secondStep == 0) || (thirdStep == 2 && secondStep == 3)
                        || (thirdStep == 3 && secondStep == 2)) {
                        continue;
                    }
                }
            }

            if (simulator.IsValidMove(playerId, move)) {
                moves.push_back(move);
            }
        } while (move.GetNextId());
        return moves;
    };

    const std::vector<int> enemyIds = std::invoke([&rootTick]() {
        const auto distance2 = [](int x, int y) -> float { return static_cast<float>(std::max(x, y) - std::min(x, y)); };
        const auto distance = [&distance2](int x1, int y1, int x2, int y2) -> float { return distance2(x1, x2) + distance2(y1, y2); };

        std::vector<int> ret;
        for (const auto& v : rootTick.mEnemyVampires) {
            if (distance(rootTick.mMe.mX, rootTick.mMe.mY, v.mX, v.mY) <= 9) {
                ret.push_back(v.mId);
            }
        }
        return ret;
    });

    if (enemyIds.empty()) {
        std::cerr << "Move restriction heuristic: enemies are too far away" << std::endl;
        return;
    }

    Simulator simulator(mGameDescription);
    for (auto& child : mLevels[1]) {
        const ActionSequence myMove(child.mAction);

        // see move count if every enemy vampire places a bomb to their position in the first tick (same as our first move)
        {
            simulator.SetState(rootTick);
            simulator.SetVampireMove(rootTick.mMe.mId, myMove.GetAnswer());

            const Answer bombingMove { true, {} };
            for (const auto& id : enemyIds) {
                simulator.SetVampireMove(id, bombingMove);
            }

            const auto desiredFuturePosition = SimulateMove(rootTick.mMe.mX, rootTick.mMe.mY, myMove);
            const auto [futureTick, points] = simulator.Tick();
            if (desiredFuturePosition.x == futureTick.mMe.mX && desiredFuturePosition.y == futureTick.mMe.mY) {
                child.mRestrictionScore += 1.F; // reward uninterruptible moves
            }
        }

        // calculate future positions for the enemies
        std::vector<std::vector<ActionSequence>> enemyMoves(enemyIds.size());
        if (myMove.IsGrenade()) {
            // "simulate" grenade to block enemy movement
            auto& myGrenade = rootTick.mGrenades.emplace_back();
            myGrenade.mId = rootTick.mMe.mId;
            myGrenade.mTick = 5;
            myGrenade.mRange = rootTick.mMe.mGrenadeRange;
            myGrenade.mX = rootTick.mMe.mX;
            myGrenade.mY = rootTick.mMe.mY;
        }

        for (size_t enemyIndex = 0; enemyIndex < enemyIds.size(); ++enemyIndex) {
            enemyMoves[enemyIndex] = GetMovesWithoutBombing(rootTick, enemyIds[enemyIndex]);
        }

        if (myMove.IsGrenade()) {
            rootTick.mGrenades.pop_back();
        }

        size_t worstMoveCount = std::numeric_limits<size_t>::max();

        bool calculationDone = false;
        std::vector<size_t> moveIndices(enemyIds.size(), 0);

        while (!calculationDone) {
            simulator.SetState(rootTick);
            simulator.SetVampireMove(rootTick.mMe.mId, myMove.GetAnswer());

            for (size_t enemyIndex = 0; enemyIndex < enemyIds.size(); ++enemyIndex) {
                const auto enemyId = enemyIds[enemyIndex];
                simulator.SetVampireMove(enemyId, enemyMoves[enemyIndex][moveIndices[enemyIndex]].GetAnswer());
            }

            auto [futureTick, points] = simulator.Tick();

            // place bombs as if the enemies would place it in the next turn
            for (const auto enemyId : enemyIds) {
                const auto vampireIt = std::find_if(
                    std::cbegin(futureTick.mEnemyVampires), std::cend(futureTick.mEnemyVampires), [&enemyId](const Vampire& v) { return v.mId == enemyId; });

                if (vampireIt != std::cend(futureTick.mEnemyVampires)) {
                    auto& grenade = futureTick.mGrenades.emplace_back();
                    grenade.mTick = 5;
                    grenade.mRange = 2;
                    grenade.mId = enemyId;
                    grenade.mX = vampireIt->mX;
                    grenade.mY = vampireIt->mY;
                }
            }

            const auto possibleMoveCount = GetMovesWithoutBombing(futureTick, futureTick.mMe.mId).size();
            worstMoveCount = std::min(possibleMoveCount, worstMoveCount);

            ++moveIndices[0];
            for (int i = static_cast<int>(moveIndices.size() - 1); i >= 0; --i) {
                const auto& iS = static_cast<size_t>(i);

                if (moveIndices[iS] == enemyMoves[iS].size()) {
                    if (i == 0) {
                        calculationDone = true;
                        break;
                    }

                    moveIndices[iS] = 0;
                    ++moveIndices[iS - 1];
                }
            }
        }

        // calculate optimal move count
        simulator.SetState(rootTick);
        simulator.SetVampireMove(rootTick.mMe.mId, myMove.GetAnswer());
        const auto [futureTick, points] = simulator.Tick();
        const auto optimalMoveCount = GetMovesWithoutBombing(futureTick, futureTick.mMe.mId).size();

        child.mRestrictionScore += static_cast<float>(worstMoveCount) / static_cast<float>(optimalMoveCount) * 1.F;
    }

    // move down restriction scores
    for (size_t i = 2; i < mLevels.size(); ++i) {
        for (auto& node : mLevels[i]) {
            node.mRestrictionScore = static_cast<float>(i) * mLevels[i - 1][node.mParentIndex].mRestrictionScore;
        }
    }
}