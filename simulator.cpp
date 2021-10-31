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
    // 5) (e) plant grenades
    PlantGrenades(retVal);
    // 6) (f) move
    Move(retVal);

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
                Area area = GetBlowArea(grenade, state);
                affectedCells.insert(area.begin(), area.end());
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

    if (state.mMe.mGhostModeTick == 0 && affectedCells.find({ state.mMe.mX, state.mMe.mY }) != affectedCells.end()) {
        state.mMe.mHealth--;
        state.mMe.mGhostModeTick = 3;
    }

    std::vector<Vampire> survivorVampires;
    for (const auto& vampire : state.mEnemyVampires) {
        if (vampire.mGhostModeTick == 0 && affectedCells.find({ vampire.mX, vampire.mY }) != affectedCells.end()) {
            if (vampire.mHealth > 1) {
                auto& v = survivorVampires.emplace_back(vampire);
                v.mHealth--;
                v.mGhostModeTick = 3;
            }
        } else {
            survivorVampires.push_back(vampire);
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

bool Simulator::IsValidMove(int id, const Answer& move)
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

std::vector<Simulator::BlowArea> Simulator::GetBlowAreas()
{
    std::vector<Simulator::BlowArea> retVal;
    if (mGrenadesByPos.empty()) {
        for (const auto& grenade : mState.mGrenades) {
            mGrenadesByPos[{ grenade.mX, grenade.mY }] = { &grenade, false };
        }
    }
    for (const auto& [_, grenadeDesc] : mGrenadesByPos) {
        if (grenadeDesc.second) {
            continue;
        }

        std::vector<const Grenade*> grenadesToProcess;
        grenadesToProcess.push_back(grenadeDesc.first);
        Simulator::BlowArea ba;
        while (!grenadesToProcess.empty()) {
            const Grenade& grenade = *grenadesToProcess.back();
            grenadesToProcess.pop_back();

            mGrenadesByPos[{ grenade.mX, grenade.mY }].second = true;
            Simulator::Area area = GetBlowArea(grenade, mState);
            for (const auto& position : area) {
                if (position == std::pair<int, int> { grenade.mX, grenade.mY }) {
                    continue;
                }
                if (auto it = mGrenadesByPos.find(position); it != mGrenadesByPos.end()) {
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