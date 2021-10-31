#pragma once

#include "action_sequence.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <memory>
#include <random>

class MonteCarloTreeSearch {
public:
    void Run();

    MonteCarloTreeSearch()
        : mEngine(0xDEADBEEF)
    {
    }

private:
    struct TreeNode {
        int mPlayerId = -1;
        TreeNode* mParent = nullptr;
        uint16_t mSimulations = 0;
        uint16_t mWins = 0;

        std::unique_ptr<std::array<TreeNode, ActionSequence::MaxSequenceId>> mActions;

        [[nodiscard]] bool IsLeaf() const
        {
            return mActions || std::any_of(std::cbegin(*mActions), std::cend(*mActions), [](const auto& action) { return action.mSimulations == 0; });
        }

        [[nodiscard]] ActionSequence GetBestAction() const;

        bool IsGameEnded() const
        {
            return false;
        }
    };

    TreeNode& Select();
    TreeNode& Expand(TreeNode& node);
    void Simulate(TreeNode& node);
    void Update() {};

    TreeNode mRoot;
    std::mt19937 mEngine;
};
