#include "mcts.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>

void MonteCarloTreeSearch::Step()
{
    TreeNode& node = Select();
    Expand(node);
    Simulate(node);
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

MonteCarloTreeSearch::TreeNode& MonteCarloTreeSearch::Expand(TreeNode& node)
{
    if (node.IsGameEnded()) {
        return node;
    }

    const auto IsValidAction = [&node, &mGameDescription = mGameDescription](const ActionSequence::ActionSequence_t actionId) {
        // action was not taken before and it is a valid move
        if (node.mActions[actionId]) {
            return false;
        }

        const ActionSequence action(actionId);
        Simulator s(mGameDescription);
        s.SetState(node.mTickDescription);
        return s.IsValidMove(node.mPlayerId, action.GetAnswer());
    };

    static std::uniform_int_distribution<unsigned short> dist(0, ActionSequence::MaxSequenceId);
    ActionSequence::ActionSequence_t actionSequenceId = static_cast<ActionSequence::ActionSequence_t>(dist(mEngine));
    while (!IsValidAction(actionSequenceId)) {
        actionSequenceId = static_cast<ActionSequence::ActionSequence_t>(dist(mEngine));
    }

    const auto playerIt = std::find(std::cbegin(mPlayerIds), std::cend(mPlayerIds), node.mPlayerId);

    auto& newNode = node.mActions[actionSequenceId];
    newNode = std::make_unique<TreeNode>(node.mTickDescription);
    newNode->mPlayerId = playerIt == std::cend(mPlayerIds) ? mPlayerIds.front() : *(playerIt + 1);
    newNode->mActionDoneByParent = actionSequenceId;

    if (playerIt == std::cend(mPlayerIds)) {
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

void MonteCarloTreeSearch::Simulate(TreeNode& node)
{
    (void)node;
}