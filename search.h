#pragma once

#include "action_sequence.h"
#include "models.h"
#include "simulator.h"

#include <algorithm>

class Search {
public:
    Search(const TickDescription& tickDescription, const GameDescription& gameDescription, int playerId)
        : mGameDescription(gameDescription)
        , mPlayerId(playerId)
    {
        const auto score = Evaluate(tickDescription, Simulator::NewPoints { { mPlayerId, 0 } }, {});
        mLevels.reserve(10);

        const auto grenadeIt = std::find_if(
            std::cbegin(tickDescription.mGrenades), std::cend(tickDescription.mGrenades), [&playerId](const auto& x) { return x.mId == playerId; });

        const ActionSequence action(Answer { grenadeIt != std::cend(tickDescription.mGrenades), {} });

        mLevels.emplace_back().emplace_back(std::numeric_limits<uint16_t>::max(), tickDescription, score, action.GetId());
    }

    void CalculateNextLevel();

    Answer GetBestMove();

    // private:
    struct TreeNode {
        TreeNode(uint16_t parent, TickDescription td, float score, ActionSequence::ActionSequence_t action)
            : mParentIndex(parent)
            , mTickDescription(std::move(td))
            , mScore(score)
            , mAction(action)
        {
        }

        uint16_t mParentIndex;
        TickDescription mTickDescription;
        float mScore;
        ActionSequence::ActionSequence_t mAction = std::numeric_limits<ActionSequence::ActionSequence_t>::max();
    };

    float Evaluate(const TickDescription& tickDescription, const Simulator::NewPoints& newPoints, const Answer& move) const
    {
        const auto distance2 = [](int x, int y) -> float { return static_cast<float>(std::max(x, y) - std::min(x, y)); };

        const auto distance = [&distance2](int x1, int y1, int x2, int y2) -> float { return distance2(x1, x2) + distance2(y1, y2); };

        float batScore = 0;

        const auto myGrenade = std::find_if(std::cbegin(tickDescription.mGrenades), std::cend(tickDescription.mGrenades),
            [&tickDescription](const Grenade& g) { return g.mId == tickDescription.mMe.mId; });
        if (myGrenade != std::cend(tickDescription.mGrenades)) {
            for (const auto& bat : tickDescription.mAllBats) {
                if (distance2(bat.mX, myGrenade->mX) <= static_cast<float>(myGrenade->mRange)
                    && distance2(bat.mY, myGrenade->mY) <= static_cast<float>(myGrenade->mRange)) {
                    batScore += 12 / static_cast<float>(bat.mDensity);
                }
            }
        }

        const auto batIt = std::min_element(
            std::cbegin(tickDescription.mAllBats), std::cend(tickDescription.mAllBats), [&tickDescription, &distance](const BatSquad& x, const BatSquad& y) {
                return distance(tickDescription.mMe.mX, tickDescription.mMe.mY, x.mX, x.mY)
                    < distance(tickDescription.mMe.mX, tickDescription.mMe.mY, y.mX, y.mY);
            });

        if (batIt != std::cend(tickDescription.mAllBats)) {
            batScore += 12.F / distance(tickDescription.mMe.mX, tickDescription.mMe.mY, batIt->mX, batIt->mY);
        }

        float grenadePenalty = 0;
        const auto closeGrenadeIt = std::min_element(
            std::cbegin(tickDescription.mGrenades), std::cend(tickDescription.mGrenades), [&tickDescription, &distance](const Grenade& x, const Grenade& y) {
                return distance(tickDescription.mMe.mX, tickDescription.mMe.mY, x.mX, x.mY)
                    < distance(tickDescription.mMe.mX, tickDescription.mMe.mY, y.mX, y.mY);
            });
        if (closeGrenadeIt != std::cend(tickDescription.mGrenades)) {
            if (distance2(tickDescription.mMe.mX, closeGrenadeIt->mX) < static_cast<float>(closeGrenadeIt->mRange + 2)
                || distance2(tickDescription.mMe.mY, closeGrenadeIt->mY) < static_cast<float>(closeGrenadeIt->mRange + 2)) {
                grenadePenalty = 48.F;
            }
        }

        return newPoints.at(mPlayerId) + 0.001F * static_cast<float>(move.mSteps.size()) + batScore + grenadePenalty;
    }

    std::vector<std::vector<TreeNode>> mLevels;

    const GameDescription& mGameDescription;
    int mPlayerId;
};