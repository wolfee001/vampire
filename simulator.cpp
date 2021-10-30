#include "simulator.h"
#include <algorithm>
#include <stdexcept>

Simulator::Simulator(const GameDescription& gameDescription)
    : mGameDescription(gameDescription)
{
}

void Simulator::SetState(const TickDescription& state)
{
    mState = state;
}

void Simulator::SetVampireMove(int id, const Answer& move)
{
    mVampireMoves[id] = move;
}

TickDescription Simulator::Tick()
{
    if (mState.mRequest.mTick == -1) {
        throw std::runtime_error("Calling tick without setting state!");
    }

    if (const auto it = mVampireMoves.find(mState.mMe.mId); it != mVampireMoves.end()) {
        if (it->second.mSteps.size() > 2 && mState.mMe.mRunningShoesTick <= 0) {
            throw std::runtime_error("Stepping more than 2 but there are no running shoes!");
        }
    }

    for (const auto& element : mState.mEnemyVampires) {
        if (const auto it = mVampireMoves.find(mState.mMe.mId); it != mVampireMoves.end()) {
            if (it->second.mSteps.size() > 2 && element.mRunningShoesTick <= 0) {
                throw std::runtime_error("Stepping more than 2 but there are no running shoes!");
            }
        }
    }

    TickDescription retVal = mState;

    // Rule:
    // a) powerup show up - not simulated, comes as state
    // b) powerup pick up
    // c) blow up grenades recursively
    // d) calculate grenade damage
    // e) calculate light damage - NOT SIMULATED! (maybe later. i didn't understand the rule.)
    // f) plant grenades
    // g) move

    // Implementation:
    // 1) recalculate ticks
    RecalculateTicks(retVal);
    // 2) remove disappeared powerups
    RemoveDisappearedPowerups(retVal);
    // 3) (b) powerup pick up
    PowerupPickUp(retVal);

    mState = TickDescription();
    mVampireMoves.clear();

    return retVal;
}

void Simulator::RecalculateTicks(TickDescription& state)
{
    state.mRequest.mTick++;

    for (auto& grenade : state.mGrenades) {
        grenade.mTick--;
    }

    for (auto& powerup : state.mPowerUps) {
        if (powerup.mRemainingTick < -1) {
            powerup.mRemainingTick++;
        } else if (powerup.mRemainingTick == -1) {
            powerup.mRemainingTick = 10;
        } else {
            powerup.mRemainingTick--;
        }
    }

    state.mMe.mRunningShoesTick = std::max(state.mMe.mRunningShoesTick - 1, 0);
    for (auto& vampire : state.mEnemyVampires) {
        vampire.mRunningShoesTick = std::max(vampire.mRunningShoesTick - 1, 0);
    }
}

void Simulator::RemoveDisappearedPowerups(TickDescription& state)
{
    state.mPowerUps.erase(
        std::remove_if(state.mPowerUps.begin(), state.mPowerUps.end(), [](const auto& element) { return element.mRemainingTick == 0; }), state.mPowerUps.end());
}

void Simulator::PowerupPickUp(TickDescription& state)
{
    std::vector<Vampire*> vampRefs;
    vampRefs.push_back(&state.mMe);
    for (auto& element : state.mEnemyVampires) {
        vampRefs.push_back(&element);
    }

    std::vector<PowerUp> survivors;

    for (const auto& pu : state.mPowerUps) {
        bool shouldDelete = false;
        for (auto* vampire : vampRefs) {
            if (vampire->mX == pu.mX && vampire->mY == pu.mY) {
                shouldDelete = true;
                switch (pu.mType) {
                case PowerUp::Type::Battery: {
                    vampire->mGrenadeRange++;
                    break;
                }
                case PowerUp::Type::Grenade: {
                    vampire->mPlacableGrenades++;
                    break;
                }
                case PowerUp::Type::Shoe: {
                    vampire->mRunningShoesTick += mGameDescription.mMapSize * 2;
                    break;
                }
                case PowerUp::Type::Tomato: {
                    vampire->mHealth = std::min(3, vampire->mHealth + 1);
                    break;
                }
                default:
                    throw std::runtime_error("Unhandled powerup type!");
                }
            }
        }
        if (!shouldDelete) {
            survivors.push_back(pu);
        }
    }

    state.mPowerUps = survivors;
}