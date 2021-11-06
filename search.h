#pragma once

#include "action_sequence.h"
#include "models.h"
#include "simulator.h"

#include <algorithm>
#include <chrono>

class Search {
public:
    Search(const TickDescription& tickDescription, const GameDescription& gameDescription, int playerId)
        : mGameDescription(gameDescription)
        , mPlayerId(playerId)
    {
        const auto heuristicScore = Evaluate(tickDescription, Simulator::NewPoints { { mPlayerId, 0.F } }, {});
        mLevels.reserve(10);

        const auto grenadeIt = std::find_if(
            std::cbegin(tickDescription.mGrenades), std::cend(tickDescription.mGrenades), [&playerId](const auto& x) { return x.mId == playerId; });

        const ActionSequence action(Answer { grenadeIt != std::cend(tickDescription.mGrenades), {} });

        mLevels.emplace_back().emplace_back(std::numeric_limits<uint32_t>::max(), tickDescription, 0.F, heuristicScore, action.GetId());

        mTomatoSafePlay = tickDescription.mMe.mHealth == 3
            && std::find_if(std::cbegin(tickDescription.mPowerUps), std::cend(tickDescription.mPowerUps),
                   [](const PowerUp& powerup) { return powerup.mType == PowerUp::Type::Tomato; })
                != std::cend(tickDescription.mPowerUps)
            && std::find_if(
                   std::cbegin(tickDescription.mEnemyVampires), std::cend(tickDescription.mEnemyVampires), [](const Vampire& v) { return v.mHealth == 1; })
                != std::cend(tickDescription.mEnemyVampires);
    }

    void SetBombSequence(std::vector<pos_t> sequence)
    {
        mBombSequence = std::move(sequence);
    }

    void SetPathSequence(std::vector<pos_t> sequence)
    {
        mPathSequence = std::move(sequence);
    }

    void SetPhase(phase_t phase)
    {
        mPhase = phase;
    }

    void SetAvoidStay(bool avoidstay)
    {
        mAvoidStay = avoidstay;
    }

    bool CalculateNextLevel(std::chrono::time_point<std::chrono::steady_clock> deadline);

    Answer GetBestMove();

    // private:
    struct TreeNode {
        TreeNode(uint32_t parent, TickDescription td, float permanentScore, float heuristicScore, ActionSequence::ActionSequence_t action)
            : mParentIndex(parent)
            , mTickDescription(std::move(td))
            , mPermanentScore(permanentScore)
            , mHeuristicScore(heuristicScore)
            , mAction(action)
        {
        }

        uint32_t mParentIndex;
        TickDescription mTickDescription;
        float mPermanentScore;
        float mHeuristicScore;
        ActionSequence::ActionSequence_t mAction = std::numeric_limits<ActionSequence::ActionSequence_t>::max();
    };

    float Evaluate(const TickDescription& tickDescription, const Simulator::NewPoints& newPoints, const Answer& move) const;

    std::vector<std::vector<TreeNode>> mLevels;

    const GameDescription& mGameDescription;
    int mPlayerId;
    bool mTomatoSafePlay = false;

    // usual magic stuff
    phase_t mPhase = NONE;
    bool mAvoidStay;
    std::vector<pos_t> mBombSequence;
    std::vector<pos_t> mPathSequence;
};
