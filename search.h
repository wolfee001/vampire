#pragma once

#include "models.h"
#include "simulator.h"

class Search {
public:
    Search(const TickDescription& tickDescription, const GameDescription& gameDescription, int playerId)
        : mGameDescription(gameDescription)
        , mPlayerId(playerId)
    {
        const auto score = Evaluate(tickDescription, Simulator::NewPoints { { mPlayerId, 0 } });
        mLevels.emplace_back().emplace_back(std::numeric_limits<uint16_t>::max(), tickDescription, score);
    }

    void CalculateNextLevel();

    Answer GetBestMove()
    {
        return {};
    }

    // private:
    struct TreeNode {
        TreeNode(uint16_t parent, TickDescription td, float score)
            : mParentIndex(parent)
            , mTickDescription(std::move(td))
            , mScore(score)
        {
        }

        uint16_t mParentIndex;
        TickDescription mTickDescription;
        float mScore;
    };

    float Evaluate(const TickDescription& tickDescription, const Simulator::NewPoints& newPoints) const
    {
        (void)tickDescription;
        return newPoints.at(mPlayerId);
    }

    std::vector<std::vector<TreeNode>> mLevels;

    const GameDescription& mGameDescription;
    int mPlayerId;
};