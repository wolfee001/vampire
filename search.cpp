#include "search.h"
#include "action_sequence.h"

void Search::CalculateNextLevel()
{
    mLevels.resize(mLevels.size() + 1);

    const auto& currentLevel = mLevels[mLevels.size() - 2];
    auto& nextLevel = mLevels.back();
    nextLevel.reserve(currentLevel.size() * 6);

    Simulator simulator(mGameDescription);

    for (size_t nodeIndex = 0; nodeIndex < currentLevel.size(); ++nodeIndex) {
        const TreeNode& node = currentLevel[nodeIndex];
        simulator.SetState(node.mTickDescription);

        for (ActionSequence::ActionSequence_t i = 0; i <= ActionSequence::MaxSequenceId; ++i) {
            const ActionSequence action(i);
            if (action.GetNumberOfSteps() == 3 && node.mTickDescription.mMe.mRunningShoesTick == 0) {
                continue;
            }
            if (action.IsGrenade() && node.mTickDescription.mMe.mPlacableGrenades == 0) {
                continue;
            }

            const Answer move = action.GetAnswer();
            if (!simulator.IsValidMove(mPlayerId, move)) {
                continue;
            }
            simulator.SetVampireMove(mPlayerId, move);
            const auto [tickDescription, newPoints] = simulator.Tick();
            simulator.SetState(node.mTickDescription);

            const auto score = Evaluate(tickDescription, newPoints);

            nextLevel.emplace_back(nodeIndex, tickDescription, score);
        }
    }
}