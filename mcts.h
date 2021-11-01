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
        : mEngine(static_cast<unsigned long long>(0xDEADBEEF))
        , mRoot(tickDescription)
        , mGameDescription(gameDescription)
        , mPlayerIds(playerIds)
    {
        mRoot.mPlayerId = mPlayerIds.front();
        mRoot.mTickDescription = tickDescription;
        mRoot.CalculatePossibleMoves(gameDescription);
    }

    void Step();

private:
    struct TreeNode {
        explicit TreeNode(TickDescription tickDescription)
            : mTickDescription(std::move(tickDescription))
        {
        }

        TreeNode(const TreeNode& tn)
            : mPlayerId(tn.mPlayerId)
            , mParent(tn.mParent)
            , mTickDescription(tn.mTickDescription)
            , mSimulations(tn.mSimulations)
            , mWins(tn.mWins)
            , mActionDoneByParent(tn.mActionDoneByParent)
            , mPossibleMoves(tn.mPossibleMoves)
        {
            // mActions are not copied!
        }

        void CalculatePossibleMoves(const GameDescription& gameDescription);

        ActionSequence::ActionSequence_t GetRandomMove(std::mt19937& engine)
        {
            std::uniform_int_distribution<size_t> dist(0, mPossibleMoves.size() - 1);
            auto it = std::cbegin(mPossibleMoves);
            std::advance(it, static_cast<long>(dist(engine)));
            const auto ret = *it;
            mPossibleMoves.erase(it);
            return ret;
        }

        int mPlayerId = -1;
        TreeNode* mParent = nullptr;
        TickDescription mTickDescription;
        uint16_t mSimulations = 0;
        uint16_t mWins = 0;
        ActionSequence::ActionSequence_t mActionDoneByParent = std::numeric_limits<ActionSequence::ActionSequence_t>::max();

        std::array<std::unique_ptr<TreeNode>, ActionSequence::MaxSequenceId> mActions;
        std::set<ActionSequence::ActionSequence_t> mPossibleMoves;

        [[nodiscard]] bool IsLeaf() const
        {
            return !mPossibleMoves.empty();
        }

        [[nodiscard]] ActionSequence GetBestAction() const;

        bool IsGameEnded() const
        {
            return false;
        }
    };

    enum GameState { WIN, DRAW, LOSE };

    TreeNode& Select();
    TreeNode& Expand(TreeNode& node);
    GameState Simulate(TreeNode& node);
    void Update() {};

    std::mt19937 mEngine;
    TreeNode mRoot;
    const GameDescription& mGameDescription;
    std::vector<int> mPlayerIds;
};
