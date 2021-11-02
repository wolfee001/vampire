#include "mcts.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>

void MonteCarloTreeSearch::Step()
{
    TreeNode& node = Select();
    TreeNode& expandedNode = Expand(node);
    const auto result = Simulate(expandedNode);
    Update(expandedNode, result);
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

        newNode->mTickDescription = simulator.Tick().first; // we moved the all players, lets tick()!
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

    Simulator::NewPoints allPoints;
    TickDescription currentTick = node.mTickDescription;
    TreeNode currentMove = node;

    constexpr size_t numberOfTurnsToSimulate = 15;

    for (size_t i = 0, max = numberOfTurnsToSimulate * 4 - alreadyMadeMoves; i < max; ++i) {
        if (i != 0 && currentMove.mPlayerId == mPlayerIds.front()) {
            simulator.SetState(currentTick);
            Simulator::NewPoints newPoints;
            std::tie(currentTick, newPoints) = simulator.Tick();
            for (const auto& [vId, point] : newPoints) {
                allPoints[vId] += point;
            }
            currentMove.mTickDescription = currentTick;
        }

        currentMove.mPossibleMoves.clear();
        currentMove.CalculatePossibleMoves(mGameDescription);
        const auto actionSequenceId = currentMove.GetRandomMove(mEngine);
        simulator.SetVampireMove(currentMove.mPlayerId, ActionSequence(actionSequenceId).GetAnswer());
        currentMove.mPlayerId = getNextPlayerId(currentMove.mPlayerId);
    }

    const auto maxIt = std::max_element(
        std::begin(allPoints), std::end(allPoints), [](const std::pair<int, float>& x, const std::pair<int, float>& y) { return x.second < y.second; });

    if (maxIt->second != 0 && (maxIt->first == mPlayerIds.front() || allPoints[mPlayerIds.front()] == maxIt->second)) {
        return GameState::WIN;
    } else {
        return GameState::LOSE;
    }
}

void MonteCarloTreeSearch::Update(TreeNode& node, const GameState& state)
{
    TreeNode* currentNode = &node;
    while (currentNode != nullptr) {
        currentNode->mSimulations++;
        if (currentNode->mPlayerId == mPlayerIds.front() && state == GameState::WIN) {
            currentNode->mWins++;
        }

        if (currentNode->mPlayerId != mPlayerIds.front() && state != GameState::WIN) {
            currentNode->mWins++;
        }

        currentNode = currentNode->mParent;
    }
}