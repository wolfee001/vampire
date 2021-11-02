#include "simulator.h"
#include "models.h"
#include <algorithm>
#include <cstddef>
#include <exception>
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

std::pair<TickDescription, Simulator::NewPoints> Simulator::Tick()
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
    mNewPoints = {};
    mNewPoints.emplace(mState.mMe.mId, 0.F);
    for (const auto& v : mState.mEnemyVampires) {
        mNewPoints.emplace(v.mId, 0.F);
    }

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
    // 5) (e) plant grenades
    PlantGrenades(retVal);
    // 6) (f) move
    Move(retVal);

    mState = TickDescription();
    mVampireMoves.clear();

    return std::make_pair(retVal, mNewPoints);
}

void Simulator::RecalculateTicks(TickDescription& state)
{
    state.mRequest.mTick++;
    /*
        if (state.mRequest.mTick == mGameDescription.mMaxTick) {
            if (state.mMe.mHealth > 0) {
                mNewPoints[state.mMe.mId] += 144.;
            }
            for (const auto& v : state.mEnemyVampires) {
                if (v.mHealth > 0) {
                    mNewPoints[v.mId] += 144.;
                }
            }
        }
    */
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
    state.mMe.mGhostModeTick = std::max(state.mMe.mGhostModeTick - 1, 0);
    for (auto& vampire : state.mEnemyVampires) {
        vampire.mRunningShoesTick = std::max(vampire.mRunningShoesTick - 1, 0);
        vampire.mGhostModeTick = std::max(vampire.mGhostModeTick - 1, 0);
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
                mNewPoints[vampire->mId] += 48;
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
    const auto areas = GetBlowAreas(true);

    state.mGrenades.erase(std::remove_if(state.mGrenades.begin(), state.mGrenades.end(),
                              [&areas](const auto& grenade) {
                                  for (const auto& area : areas) {
                                      if (area.mArea.find({ grenade.mX, grenade.mY }) != area.mArea.end()) {
                                          return true;
                                      }
                                  }
                                  return false;
                              }),
        state.mGrenades.end());

    std::vector<BatSquad> survivorBats;

    for (const auto& bat : state.mAllBats) {
        int injured = 0;

        for (const auto& area : areas) {
            if (area.mArea.find({ bat.mX, bat.mY }) != area.mArea.end()) {
                injured++;
                if (bat.mDensity <= injured) {
                    for (const auto& vId : area.mVampireIds) {
                        mNewPoints[vId] += 12.F / static_cast<float>(area.mVampireIds.size());
                    }
                }
            }
        }

        if (bat.mDensity > injured) {
            survivorBats.emplace_back(bat).mDensity -= injured;
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
    /*
        if (state.mMe.mGhostModeTick == 0) {
            const auto areaIt = std::find_if(std::cbegin(areas), std::cend(areas), [&state](const auto& area) {
                return area.mArea.find({ state.mMe.mX, state.mMe.mY }) != area.mArea.end();
            });
            if (areaIt != std::cend(areas)) {
                state.mMe.mHealth--;
                state.mMe.mGhostModeTick = 3;

                const auto meIt = std::find(std::cbegin(areaIt->mVampireIds), std::cend(areaIt->mVampireIds), state.mMe.mId);
                const bool meInTheArea = meIt == std::cend(areaIt->mVampireIds);

                for (const auto& vId : areaIt->mVampireIds) {
                    if (vId == state.mMe.mId) {
                        // i get penalized for injuring myself
                        mNewPoints[vId] -= 48.F / static_cast<float>(areaIt->mVampireIds.size() - (meInTheArea ? 1 : 0));
                    } else {
                        mNewPoints[vId] += 48.F / static_cast<float>(areaIt->mVampireIds.size() - (meInTheArea ? 1 : 0));
                    }
                }
            }
        }
    */
    std::vector<Vampire> survivorVampires;

    std::vector<Vampire*> vampires = { &state.mMe };
    for (auto& vampire : state.mEnemyVampires) {
        vampires.push_back(&vampire);
    }

    for (const auto& vampirePtr : vampires) {
        auto& vampire = *vampirePtr;

        if (vampire.mGhostModeTick == 0 && vampire.mHealth > 0) {

            bool isDead = false;
            bool alreadyDamaged = false;
            std::vector<int> vampiresDamaging;

            for (const auto& area : areas) {
                if (area.mArea.find({ vampire.mX, vampire.mY }) != area.mArea.end()) {
                    isDead = vampire.mHealth == 1;

                    if (!alreadyDamaged && vampire.mHealth >= 2 && vampire.mGhostModeTick == 0) {
                        if (vampire.mId != state.mMe.mId) {
                            auto& v = survivorVampires.emplace_back(vampire);
                            v.mHealth--;
                            v.mGhostModeTick = 3;
                        } else {
                            state.mMe.mHealth--;
                            state.mMe.mGhostModeTick = 3;
                        }
                        alreadyDamaged = true;
                    }

                    vampiresDamaging.insert(std::end(vampiresDamaging), std::cbegin(area.mVampireIds), std::cend(area.mVampireIds));
                }
            }

            if ((areas.empty() || vampiresDamaging.empty()) && vampire.mId != state.mMe.mId) {
                survivorVampires.emplace_back(vampire);
            }

            if (isDead) {
                for (const auto& vId : vampiresDamaging) {
                    if (vId != vampire.mId) {
                        mNewPoints[vId] += 96.F / static_cast<float>(vampiresDamaging.size());
                    } else {
                        mNewPoints[vId] -= 96.F / static_cast<float>(vampiresDamaging.size());
                    }
                }
            }

            for (const auto& vId : vampiresDamaging) {
                if (vId != vampire.mId) {
                    mNewPoints[vId] += 48.F / static_cast<float>(vampiresDamaging.size());
                } else {
                    mNewPoints[vId] -= 48.F / static_cast<float>(vampiresDamaging.size());
                }
            }
        } else {
            if (vampire.mId != state.mMe.mId) {
                survivorVampires.push_back(vampire);
            }
        }
    }
    state.mEnemyVampires = survivorVampires;
}

void Simulator::PlantGrenades(TickDescription& state)
{
    std::vector<Vampire*> vampRefs;
    vampRefs.push_back(&state.mMe);
    for (auto& element : state.mEnemyVampires) {
        vampRefs.push_back(&element);
    }

    for (const auto& vampire : vampRefs) {
        if (const auto it = mVampireMoves.find(vampire->mId); it != mVampireMoves.end()) {
            if (it->second.mPlaceGrenade) {
                if (vampire->mGhostModeTick != 0) {
                    continue;
                }
                int placedGrenades = 0;
                for (const auto& grenade : state.mGrenades) {
                    if (grenade.mId == vampire->mId) {
                        placedGrenades++;
                    }
                }
                if (placedGrenades < vampire->mPlacableGrenades) {
                    state.mGrenades.push_back({ vampire->mId, vampire->mX, vampire->mY, 5, vampire->mGrenadeRange });
                }
            }
        }
    }
}

void Simulator::Move(TickDescription& state)
{
    std::set<std::pair<int, int>> restricted;

    for (int x = 0; x < mGameDescription.mMapSize; ++x) {
        for (int y = 0; y < mGameDescription.mMapSize; ++y) {
            if (x == 0 || y == 0 || x == mGameDescription.mMapSize - 1 || y == mGameDescription.mMapSize - 1 || (!(x % 2) && !(y % 2))) {
                restricted.emplace(x, y);
            }
        }
    }
    for (const auto& grenade : state.mGrenades) {
        restricted.emplace(grenade.mX, grenade.mY);
    }
    for (const auto& bat : state.mAllBats) {
        restricted.emplace(bat.mX, bat.mY);
    }

    std::vector<Vampire*> vampRefs;
    vampRefs.push_back(&state.mMe);
    for (auto& element : state.mEnemyVampires) {
        vampRefs.push_back(&element);
    }

    for (const auto& vampire : vampRefs) {
        if (const auto it = mVampireMoves.find(vampire->mId); it != mVampireMoves.end()) {
            for (const auto& d : it->second.mSteps) {
                if (d == 'U') {
                    if (restricted.find({ vampire->mX, vampire->mY - 1 }) != restricted.end()) {
                        break;
                    }
                    vampire->mY--;
                } else if (d == 'R') {
                    if (restricted.find({ vampire->mX + 1, vampire->mY }) != restricted.end()) {
                        break;
                    }
                    vampire->mX++;
                } else if (d == 'D') {
                    if (restricted.find({ vampire->mX, vampire->mY + 1 }) != restricted.end()) {
                        break;
                    }
                    vampire->mY++;
                } else if (d == 'L') {
                    if (restricted.find({ vampire->mX - 1, vampire->mY }) != restricted.end()) {
                        break;
                    }
                    vampire->mX--;
                } else {
                    throw std::runtime_error("Invalid direction!");
                }
            }
        }
    }
}

bool Simulator::IsValidMove(int id, const Answer& move) const
{
    const Vampire* vampire = nullptr;

    if (mState.mMe.mId == id) {
        vampire = &mState.mMe;
    } else {
        for (auto& element : mState.mEnemyVampires) {
            if (element.mId == id) {
                vampire = &element;
                break;
            }
        }
    }

    if (vampire == nullptr) {
        throw std::runtime_error("Invalid id!");
    }

    if (move.mPlaceGrenade) {
        if (vampire->mGhostModeTick != 0) {
            return false;
        }
        int placedGrenades = 0;
        for (const auto& grenade : mState.mGrenades) {
            if (grenade.mId == vampire->mId) {
                placedGrenades++;
            }
        }
        if (placedGrenades >= vampire->mPlacableGrenades) {
            return false;
        }
    }

    if (move.mSteps.size() > 3) {
        return false;
    }

    if (move.mSteps.size() > 2 && vampire->mRunningShoesTick == 0) {
        return false;
    }

    int x = vampire->mX;
    int y = vampire->mY;
    for (const auto& d : move.mSteps) {
        if (d == 'U') {
            y--;
        } else if (d == 'R') {
            x++;
        } else if (d == 'D') {
            y++;
        } else if (d == 'L') {
            x--;
        } else {
            throw std::runtime_error("Invalid direction!");
        }

        if (x == 0 || y == 0 || x == mGameDescription.mMapSize - 1 || y == mGameDescription.mMapSize - 1 || (!(x % 2) && !(y % 2))) {
            return false;
        }
    }

    return true;
}

std::vector<Simulator::BlowArea> Simulator::GetBlowAreas(const bool blowNow /*= false*/)
{
    std::vector<Simulator::BlowArea> retVal;
    std::map<std::pair<int, int>, std::pair<const Grenade*, bool>> grenadesByPos;
    for (const auto& grenade : mState.mGrenades) {
        grenadesByPos[{ grenade.mX, grenade.mY }] = { &grenade, false };
    }
    for (const auto& [_, grenadeDesc] : grenadesByPos) {
        if (blowNow && grenadeDesc.first->mTick != 1) {
            continue;
        }

        if (grenadeDesc.second) {
            continue;
        }

        std::vector<const Grenade*> grenadesToProcess;
        grenadesToProcess.push_back(grenadeDesc.first);
        Simulator::BlowArea ba;
        while (!grenadesToProcess.empty()) {
            const Grenade& grenade = *grenadesToProcess.back();
            grenadesToProcess.pop_back();

            grenadesByPos[{ grenade.mX, grenade.mY }].second = true;
            Simulator::Area area = GetBlowArea(grenade, mState);
            for (const auto& position : area) {
                if (position == std::pair<int, int> { grenade.mX, grenade.mY }) {
                    continue;
                }
                if (auto it = grenadesByPos.find(position); it != grenadesByPos.end()) {
                    if (!it->second.second) {
                        grenadesToProcess.push_back(it->second.first);
                    }
                }
            }
            ba.mArea.insert(area.begin(), area.end());
            ba.mTickCount = ba.mTickCount == -1 ? grenade.mTick : std::min(ba.mTickCount, grenade.mTick);
            ba.mVampireIds.insert(grenade.mId);
        }
        retVal.push_back(ba);
    }
    return retVal;
}

Simulator::Area Simulator::GetBlowArea(const Grenade& grenade, const TickDescription& state)
{
    Area area;
    const auto checkExplosion = [&state, &mGameDescription = mGameDescription, &area](const int px, const int py) {
        if (px == 0 || py == 0 || px == mGameDescription.mMapSize - 1 || py == mGameDescription.mMapSize - 1 || (!(px % 2) && !(py % 2))) {
            return false;
        }
        area.insert({ px, py });
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
    return area;
}