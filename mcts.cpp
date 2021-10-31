#include "mcts.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>

ActionSequence MonteCarloTreeSearch::TreeNode::GetBestAction() const
{
    const auto UpperConfidenceBound = [](const TreeNode& node) {
        constexpr float C = 1.4142135623730f;
        return (static_cast<float>(node.mWins) / node.mSimulations) + C * std::sqrt(logf(node.mParent->mSimulations)) / (1.f + node.mSimulations);
    };

    const auto it = std::max_element(std::cbegin(*mActions), std::cend(*mActions),
        [&UpperConfidenceBound](const TreeNode& x, const TreeNode& y) { return UpperConfidenceBound(x) < UpperConfidenceBound(y); });
    return ActionSequence(static_cast<uint8_t>(std::distance(std::cbegin(*mActions), it)));
}

void MonteCarloTreeSearch::Run()
{
    while (true) {
        TreeNode& node = Select();
        Expand(node);
        Simulate(node);
        Update();
    }
}

MonteCarloTreeSearch::TreeNode& MonteCarloTreeSearch::Select()
{
    TreeNode* current = &mRoot;
    while (!current->IsLeaf()) {
        const auto next = current->GetBestAction();
        current = &(*(current->mActions))[next.GetId()];
    }
    return *current;
}

MonteCarloTreeSearch::TreeNode& MonteCarloTreeSearch::Expand(TreeNode& node)
{
    if (!node.IsGameEnded()) {
        return node;
    }

    if (!node.mActions) {
        node.mActions = std::make_unique<std::array<TreeNode, ActionSequence::MaxSequenceId>>();
    }

    static std::uniform_int_distribution<ActionSequence::ActionSequence_t> dist(0, ActionSequence::MaxSequenceId);

    const auto IsValidAction = [](const ActionSequence::ActionSequence_t) { return true; };

    ActionSequence::ActionSequence_t actionSequenceId = dist(mEngine);
    while (!IsValidAction(actionSequenceId)) {
        actionSequenceId = dist(mEngine);
    }

    return (*node.mActions)[actionSequenceId];
}

void MonteCarloTreeSearch::Simulate(TreeNode& node)
{
    (void)node;
}