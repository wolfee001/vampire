#include "mcts.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>

void MonteCarloTreeSearch::Step()
{
    TreeNode& node = Select();
    TreeNode& expandedNode = Expand(node);
    Simulate(expandedNode);
    Update();
}

MonteCarloTreeSearch::TreeNode& MonteCarloTreeSearch::Select()
{
    TreeNode* current = &mRoot;
    while (!current->IsLeaf()) {
        const auto next = current->GetBestAction();
        current = current->mActions[next.GetId()].get();
    }
    return *current;
}

ActionSequence MonteCarloTreeSearch::TreeNode::GetBestAction() const
{
    const auto UpperConfidenceBound = [](const TreeNode& node) {
        constexpr float C = 1.4142135623730f;
        return (static_cast<float>(node.mWins) / node.mSimulations) + C * std::sqrt(logf(node.mParent->mSimulations)) / (1.f + node.mSimulations);
    };

    const auto it = std::max_element(
        std::cbegin(mActions), std::cend(mActions), [&UpperConfidenceBound](const std::unique_ptr<TreeNode>& x, const std::unique_ptr<TreeNode>& y) {
            if (!x || !y) {
                return x < y;
            }
            return UpperConfidenceBound(*x) < UpperConfidenceBound(*y);
        });
    return ActionSequence(static_cast<uint8_t>(std::distance(std::cbegin(mActions), it)));
}

void MonteCarloTreeSearch::TreeNode::CalculatePossibleMoves(const GameDescription& gameDescription)
{
    Simulator simulator(gameDescription);
    simulator.SetState(mTickDescription);

    for (ActionSequence::ActionSequence_t i = 0; i <= ActionSequence::MaxSequenceId; ++i) {
        const ActionSequence action(i);
        if (simulator.IsValidMove(mPlayerId, action.GetAnswer())) {
            mPossibleMoves.insert(i);
        }
    }
}

MonteCarloTreeSearch::TreeNode& MonteCarloTreeSearch::Expand(TreeNode& node)
{
    if (node.IsGameEnded()) {
        return node;
    }

    const ActionSequence::ActionSequence_t actionSequenceId = node.GetRandomMove(mEngine);

    const auto playerIt = std::find(std::cbegin(mPlayerIds), std::cend(mPlayerIds), node.mPlayerId);

    auto& newNode = node.mActions[actionSequenceId];
    newNode = std::make_unique<TreeNode>(node.mTickDescription);
    newNode->mPlayerId = playerIt == std::cend(mPlayerIds) - 1 ? mPlayerIds.front() : *(playerIt + 1);
    newNode->mActionDoneByParent = actionSequenceId;
    newNode->CalculatePossibleMoves(mGameDescription);
    newNode->mParent = &node;

    if (node.mParent != nullptr && playerIt == std::begin(mPlayerIds)) {
        Simulator simulator(mGameDescription);
        simulator.SetState(node.mTickDescription);

        TreeNode* current = newNode.get();
        TreeNode* parent = &node;

        // 4th player
        simulator.SetVampireMove(parent->mPlayerId, ActionSequence(current->mActionDoneByParent).GetAnswer());

        // 3rd player
        current = parent;
        parent = parent->mParent;
        simulator.SetVampireMove(parent->mPlayerId, ActionSequence(current->mActionDoneByParent).GetAnswer());

        // 2nd player
        current = parent;
        parent = parent->mParent;
        simulator.SetVampireMove(parent->mPlayerId, ActionSequence(current->mActionDoneByParent).GetAnswer());

        // 1st player
        current = parent;
        parent = parent->mParent;
        simulator.SetVampireMove(parent->mPlayerId, ActionSequence(current->mActionDoneByParent).GetAnswer());

        newNode->mTickDescription = simulator.Tick(); // we moved the all players, lets tick()!
    }

    return *newNode;
}

MonteCarloTreeSearch::GameState MonteCarloTreeSearch::Simulate(TreeNode& node)
{
    Simulator simulator(mGameDescription);
    simulator.SetState(node.mTickDescription);

    TreeNode* current = &node;
    TreeNode* parent = node.mParent;

    size_t alreadyMadeMoves = 0;

    while (current->mPlayerId != mPlayerIds.front()) {
        simulator.SetVampireMove(parent->mPlayerId, ActionSequence(current->mActionDoneByParent).GetAnswer());
        current = parent;
        parent = parent->mParent;
        ++alreadyMadeMoves;
    }

    const auto getNextPlayerId = [&mPlayerIds = mPlayerIds](int playerId) -> int {
        const auto playerIt = std::find(std::cbegin(mPlayerIds), std::cend(mPlayerIds), playerId);
        return mPlayerIds[(static_cast<size_t>(std::distance(std::cbegin(mPlayerIds), playerIt)) + 1) % mPlayerIds.size()];
    };

    TickDescription currentTick = node.mTickDescription;
    TreeNode currentMove = node;

    constexpr size_t numberOfTurnsToSimulate = 5;

    for (size_t i = 0, max = numberOfTurnsToSimulate * 4 - alreadyMadeMoves; i < max; ++i) {
        if (i != 0 && currentMove.mPlayerId == mPlayerIds.front()) {
            simulator.SetState(currentTick);
            currentTick = simulator.Tick();
            currentMove.mTickDescription = currentTick;
        }

        currentMove.mPossibleMoves.clear();
        currentMove.CalculatePossibleMoves(mGameDescription);
        const auto actionSequenceId = currentMove.GetRandomMove(mEngine);
        simulator.SetVampireMove(currentMove.mPlayerId, ActionSequence(actionSequenceId).GetAnswer());
        currentMove.mPlayerId = getNextPlayerId(currentMove.mPlayerId);
    }

    return GameState::WIN;
}