#pragma once

#include "action_sequence.h"
#include "models.h"
#include "simulator.h"

#include <algorithm>
#include <chrono>

class Search {
public:
    Search(const TickDescription& tickDescription, const GameDescription& gameDescription, int playerId);

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

    void SetAvoids(int avoids)
    {
        mAvoids = avoids;
    }

    void SetPreferGrenade(int prefer)
    {
        mPreferGrenade = prefer;
    }

    void SetReachDiff(int reachdiff)
    {
        mReachDiff = reachdiff;
    }

    void SetThrow(std::optional<Throw> pThrow)
    {
        mThrow = std::move(pThrow);
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
        float mRestrictionScore = 0;
        ActionSequence::ActionSequence_t mAction = std::numeric_limits<ActionSequence::ActionSequence_t>::max();
    };

    float Evaluate(const TickDescription& tickDescription, const Simulator::NewPoints& newPoints, const Answer& move, const size_t level,
        const bool printScores = false) const;

    void CalculateMoveRestrictions(const bool defense = true);

    std::vector<std::vector<TreeNode>> mLevels;

    const GameDescription& mGameDescription;
    int mPlayerId;
    bool mTomatoSafePlay = false;

    // usual magic stuff
    phase_t mPhase = NONE;
    int mAvoids = 0;
    int mPreferGrenade = 0;
    int mReachDiff = 0;
    std::vector<pos_t> mBombSequence;
    std::vector<pos_t> mPathSequence;
    pos_t mMyOriginalPos;
    std::optional<Throw> mThrow;
};
