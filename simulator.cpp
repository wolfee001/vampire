#include "simulator.h"
#include "models.h"
#include <algorithm>
#include <cstddef>
#include <set>
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
    // c) blow up grenades recursively and calculate grenade damage
    // d) calculate light damage - NOT SIMULATED! (maybe later. i didn't understand the rule.)
    // e) plant grenades
    // f) move

    // Implementation:
    // 1) recalculate ticks
    RecalculateTicks(retVal);
    // 2) remove disappeared powerups
    RemoveDisappearedPowerups(retVal);
    // 3) (b) powerup pick up
    PowerupPickUp(retVal);
    // 4) (c) blow up grenades recursively and calculate grenade damage
    BlowUpGrenades(retVal);

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

void Simulator::BlowUpGrenades(TickDescription& state)
{
    std::set<std::pair<int, int>> affectedCells;

    while (true) {
        size_t affectedCount = affectedCells.size();

        for (const auto& grenade : state.mGrenades) {
            if (grenade.mTick == 0) {
                const auto checkExplosion = [&state, &mGameDescription = mGameDescription, &affectedCells](const int px, const int py) {
                    if (px == 0 || py == 0 || px == mGameDescription.mMapSize - 1 || py == mGameDescription.mMapSize - 1 || (!(px % 2) && !(py % 2))) {
                        return false;
                    }
                    affectedCells.emplace(px, py);
                    for (const auto& bat : state.mAllBats) {
                        if (bat.mX == px && bat.mY == py) {
                            return false;
                        }
                    }
                    return true;
                };
                for (int x = 0; x < grenade.mRange + 1; ++x) {
                    if (!checkExplosion(grenade.mX + x, grenade.mY)) {
                        break;
                    }
                }
                for (int x = 0; x < grenade.mRange + 1; ++x) {
                    if (!checkExplosion(grenade.mX - x, grenade.mY)) {
                        break;
                    }
                }
                for (int y = 0; y < grenade.mRange + 1; ++y) {
                    if (!checkExplosion(grenade.mX, grenade.mY + y)) {
                        break;
                    }
                }
                for (int y = 0; y < grenade.mRange + 1; ++y) {
                    if (!checkExplosion(grenade.mX, grenade.mY - y)) {
                        break;
                    }
                }
            }
        }
        for (auto& grenade : state.mGrenades) {
            if (affectedCells.find({ grenade.mX, grenade.mY }) != affectedCells.end()) {
                grenade.mTick = 0;
            }
        }

        if (affectedCells.size() == affectedCount) {
            break;
        }
    }

    state.mGrenades.erase(
        std::remove_if(state.mGrenades.begin(), state.mGrenades.end(), [](const auto& element) { return element.mTick == 0; }), state.mGrenades.end());

    std::vector<BatSquad> survivorBats;
    for (const auto& bat : state.mAllBats) {
        if (affectedCells.find({ bat.mX, bat.mY }) != affectedCells.end()) {
            if (bat.mDensity > 1) {
                survivorBats.emplace_back(bat).mDensity--;
            }
        } else {
            survivorBats.push_back(bat);
        }
    }
    state.mAllBats = survivorBats;
    state.mBat1.clear();
    state.mBat2.clear();
    state.mBat3.clear();
    for (const auto& bat : state.mAllBats) {
        if (bat.mDensity == 1) {
            state.mBat1.push_back(bat);
        } else if (bat.mDensity == 2) {
            state.mBat2.push_back(bat);
        } else if (bat.mDensity == 3) {
            state.mBat3.push_back(bat);
        } else {
            throw std::runtime_error("Some calculation is wrong...");
        }
    }

    if (affectedCells.find({ state.mMe.mX, state.mMe.mY }) != affectedCells.end()) {
        state.mMe.mHealth--;
    }

    std::vector<Vampire> survivorVampires;
    for (const auto& vampire : state.mEnemyVampires) {
        if (affectedCells.find({ vampire.mX, vampire.mY }) != affectedCells.end()) {
            if (vampire.mHealth > 1) {
                survivorVampires.emplace_back(vampire).mHealth--;
            }
        } else {
            survivorVampires.push_back(vampire);
        }
    }
    state.mEnemyVampires = survivorVampires;
}
