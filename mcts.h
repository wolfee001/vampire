#pragma once

#include "action_sequence.h"
#include "simulator.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <memory>
#include <random>

class MonteCarloTreeSearch {
public:
    // first id in playerIds must be our own
    explicit MonteCarloTreeSearch(const TickDescription& tickDescription, const GameDescription& gameDescription, const std::vector<int>& playerIds)
        : mEngine(0xDEADBEEF)
        , mRoot(tickDescription)
        , mGameDescription(gameDescription)
        , mPlayerIds(playerIds)
    {
        mRoot.mPlayerId = mPlayerIds.front();
    }

    void Step();

private:
    struct TreeNode {
        explicit TreeNode(TickDescription tickDescription)
            : mTickDescription(std::move(tickDescription))
        {
        }

        int mPlayerId = -1;
        TreeNode* mParent = nullptr;
        TickDescription mTickDescription;
        uint16_t mSimulations = 0;
        uint16_t mWins = 0;
        ActionSequence::ActionSequence_t mActionDoneByParent = std::numeric_limits<ActionSequence::ActionSequence_t>::max();

        std::array<std::unique_ptr<TreeNode>, ActionSequence::MaxSequenceId> mActions;

        [[nodiscard]] bool IsLeaf() const
        {
            return std::any_of(std::cbegin(mActions), std::cend(mActions), [](const auto& action) -> bool { return action.get() == nullptr; });
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

    std::mt19937 mEngine;
    TreeNode mRoot;
    const GameDescription& mGameDescription;
    std::vector<int> mPlayerIds;
};
