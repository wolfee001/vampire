#include "search.h"
#include "action_sequence.h"

void Search::CalculateNextLevel()
{
    const int currentLevelIndex = static_cast<int>(mLevels.size());
    mLevels.resize(mLevels.size() + 1);

    const auto& currentLevel = mLevels[mLevels.size() - 2];
    auto& nextLevel = mLevels.back();
    nextLevel.reserve(currentLevel.size() * 3);

    Simulator simulator(mGameDescription);

    for (size_t nodeIndex = 0; nodeIndex < currentLevel.size(); ++nodeIndex) {
        const TreeNode& node = currentLevel[nodeIndex];

        TickDescription tick = node.mTickDescription;
        tick.mEnemyVampires.clear();

        simulator.SetState(tick);

        for (ActionSequence::ActionSequence_t i = 0; i <= ActionSequence::MaxSequenceId; ++i) {
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
            for (int level = currentLevelIndex - 1; level >= 1; --level) {
                searchNode = &mLevels[static_cast<size_t>(level)][searchNode->mParentIndex];
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

            size_t numberOfChildren = 0;
            mLevels[level - 1][parentIndex].mScore = 0;

            for (; mLevels[level][i].mParentIndex == parentIndex; ++i, ++numberOfChildren) {
                mLevels[level - 1][parentIndex].mScore += mLevels[level][i].mScore;
            }
            mLevels[level - 1][parentIndex].mScore /= static_cast<float>(numberOfChildren);
        }
    }

    const auto it = std::max_element(std::cbegin(mLevels[1]), std::cend(mLevels[1]), [](const TreeNode& x, const TreeNode& y) { return x.mScore < y.mScore; });

    return ActionSequence(it->mAction).GetAnswer();
}