#include "simulator.h"
#include "models.h"
#include <algorithm>
#include <bitset>
#include <boost/functional/hash.hpp>
#include <cstddef>
#include <exception>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "check.h"

bool Simulator::Area::find(int x, int y) const
{
    return mAreas[static_cast<size_t>(y) * mMapSize + static_cast<size_t>(x)];
}

void Simulator::Area::insert(int x, int y)
{
    mAreas.set(static_cast<size_t>(y) * mMapSize + static_cast<size_t>(x));
    mVectorIsValid = false;
}

void Simulator::Area::clear(int x, int y)
{
    mAreas.reset(static_cast<size_t>(y) * mMapSize + static_cast<size_t>(x));
    mVectorIsValid = false;
}

const Simulator::Area::AreaVector& Simulator::Area::getAsVector() const
{
    if (!mVectorIsValid) {
        Simulator::Area& nonConst = const_cast<Simulator::Area&>(*this);
        nonConst.mVector.reserve(23 * 23);
        for (int x = 0; x < static_cast<int>(mMapSize); ++x) {
            for (int y = 0; y < static_cast<int>(mMapSize); ++y) {
                if (find(x, y)) {
                    nonConst.mVector.emplace_back(x, y);
                }
            }
        }
        nonConst.mVectorIsValid = true;
    }

    return mVector;
}

Simulator::Simulator(const GameDescription& gameDescription)
    : mGameDescription(gameDescription)
    , mReachableArea(mGameDescription.mMapSize)
    , mThrowableArea(mGameDescription.mMapSize)
    , mLitArea(mGameDescription.mMapSize)
{
}

void Simulator::SetState(const TickDescription& state)
{
    if (mGameDescription.mMapSize != 0) {
        if (mState.mAllBats != state.mAllBats || mState.mGrenades != state.mGrenades || !mReachableArea.mAreas.any()) {
            mReachableArea = Area(mGameDescription.mMapSize);
            mThrowableArea = Area(mGameDescription.mMapSize);
            for (const auto& bat : state.mAllBats) {
                mReachableArea.insert(bat.mX, bat.mY);
                mThrowableArea.insert(bat.mX, bat.mY);
            }
            for (const auto& grenade : state.mGrenades) {
                mReachableArea.insert(grenade.mX, grenade.mY);
            }
            for (int x = 0; x < mGameDescription.mMapSize; ++x) {
                for (int y = 0; y < mGameDescription.mMapSize; ++y) {
                    if (x == 0 || y == 0 || x == mGameDescription.mMapSize - 1 || y == mGameDescription.mMapSize - 1 || (!(x % 2) && !(y % 2))) {
                        mReachableArea.insert(x, y);
                        mThrowableArea.insert(x, y);
                    }
                }
            }
            mReachableArea.mAreas.flip();
            mThrowableArea.mAreas.flip();
        }

        mLitArea = Area(mGameDescription.mMapSize);
        if (state.mRequest.mTick >= mGameDescription.mMaxTick) {
            int x = mGameDescription.mMapSize - 1;
            int y = 0;
            for (int i = 0; i < std::min(state.mRequest.mTick - (mGameDescription.mMaxTick - 1), mGameDescription.mMapSize * mGameDescription.mMapSize / 4 + 1);
                 ++i) {
                mLitArea.insert(x, y);
                mLitArea.insert(mGameDescription.mMapSize - x - 1, mGameDescription.mMapSize - y - 1);
                mLitArea.insert(y, mGameDescription.mMapSize - x - 1);
                mLitArea.insert(mGameDescription.mMapSize - y - 1, x);
                x--;
                if (x == y) {
                    y++;
                    x = mGameDescription.mMapSize - 1 - y;
                }
            }
        }
    }

    mState = state;
    mValid = true;
}

const Simulator::Area& Simulator::GetReachableArea() const
{
    return mReachableArea;
}

const Simulator::Area& Simulator::GetLitArea() const
{
    return mLitArea;
}

void Simulator::KillVampire(int id)
{
    if (mState.mRequest.mTick == -1 || !mValid) {
        CHECK(false, "Calling tick without setting state!");
    }

    if (id == mState.mMe.mId) {
        mState.mMe.mHealth = -1;
    } else {
        mState.mEnemyVampires.erase(std::remove_if(mState.mEnemyVampires.begin(), mState.mEnemyVampires.end(), [id](const auto& v) { return v.mId == id; }),
            mState.mEnemyVampires.end());
    }
}

void Simulator::SetVampireMove(int id, const Answer& move)
{
    mVampireMoves[id] = move;
}

std::pair<TickDescription, Simulator::NewPoints> Simulator::Tick()
{
    if (mState.mRequest.mTick == -1 || !mValid) {
        CHECK(false, "Calling tick without setting state!");
    }

    if (const auto it = mVampireMoves.find(mState.mMe.mId); it != mVampireMoves.end()) {
        if (it->second.mSteps.size() > 2 && mState.mMe.mRunningShoesTick <= 0) {
            CHECK(false, "Stepping more than 2 but there are no running shoes!");
        }
    }

    for (const auto& element : mState.mEnemyVampires) {
        if (const auto it = mVampireMoves.find(element.mId); it != mVampireMoves.end()) {
            if (it->second.mSteps.size() > 2 && element.mRunningShoesTick <= 0) {
                CHECK(false, "Stepping more than 2 but there are no running shoes!");
            }
        }
    }

    mValid = false;
    mNewPoints = {};
    mNewPoints.emplace(mState.mMe.mId, 0.F);
    for (const auto& v : mState.mEnemyVampires) {
        mNewPoints.emplace(v.mId, 0.F);
    }

    // Rule:
    // a) powerup show up - not simulated, comes as state
    // b) powerup pick up
    // c) blow up grenades recursively and calculate grenade damage
    // d) calculate light damage
    // e) plant grenades
    // f) throw grenades
    // g) move

    // Implementation:
    // 1) recalculate ticks
    RecalculateTicks();
    // 2) remove disappeared powerups
    RemoveDisappearedPowerups();
    // 3) (b) powerup pick up
    PowerupPickUp();
    // 4) (c) blow up grenades recursively and calculate grenade damage
    BlowUpGrenades();
    // 5) (d) calculate light damage
    HitLight();
    // 6) (e) plant grenades
    PlantGrenades();
    // 7) (f) throw grenades
    ThrowGrenades();
    // 8) (g) move
    Move();

    mVampireMoves.clear();

    return std::make_pair(mState, mNewPoints);
}

void Simulator::RecalculateTicks()
{
    mState.mRequest.mTick++;
    for (auto& grenade : mState.mGrenades) {
        grenade.mTick--;
    }

    for (auto& powerup : mState.mPowerUps) {
        if (powerup.mRemainingTick < -1) {
            powerup.mRemainingTick++;
        } else if (powerup.mRemainingTick == -1) {
            powerup.mRemainingTick = powerup.mKeepAliveHint == -1 ? 10 : powerup.mKeepAliveHint;
        } else {
            powerup.mRemainingTick--;
        }
    }

    mState.mMe.mRunningShoesTick = std::max(mState.mMe.mRunningShoesTick - 1, 0);
    mState.mMe.mGhostModeTick = std::max(mState.mMe.mGhostModeTick - 1, 0);
    for (auto& vampire : mState.mEnemyVampires) {
        vampire.mRunningShoesTick = std::max(vampire.mRunningShoesTick - 1, 0);
        vampire.mGhostModeTick = std::max(vampire.mGhostModeTick - 1, 0);
    }
}

void Simulator::RemoveDisappearedPowerups()
{
    mState.mPowerUps.erase(std::remove_if(mState.mPowerUps.begin(), mState.mPowerUps.end(), [](const auto& element) { return element.mRemainingTick == 0; }),
        mState.mPowerUps.end());
}

void Simulator::PowerupPickUp()
{
    std::vector<Vampire*> vampRefs;
    vampRefs.push_back(&mState.mMe);
    for (auto& element : mState.mEnemyVampires) {
        vampRefs.push_back(&element);
    }

    std::vector<PowerUp> survivors;

    for (const auto& pu : mState.mPowerUps) {
        bool shouldDelete = false;
        if (pu.mRemainingTick > 0) {
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
                        CHECK(false, "Unhandled powerup type!");
                    }
                }
            }
        }
        if (!shouldDelete) {
            survivors.push_back(pu);
        }
    }

    mState.mPowerUps = survivors;
}

void Simulator::BlowUpGrenades()
{
    const auto areas = GetBlowAreas(true);

    std::unordered_map<int, Vampire*> vampRefs;
    vampRefs.emplace(mState.mMe.mId, &mState.mMe);
    for (auto& element : mState.mEnemyVampires) {
        vampRefs.emplace(element.mId, &element);
    }

    for (const auto& grenade : mState.mGrenades) {
        for (const auto& area : areas) {
            if (area.mArea.find(grenade.mX, grenade.mY)) {
                mReachableArea.insert(grenade.mX, grenade.mY);
                if (const auto it = vampRefs.find(grenade.mId); it != std::cend(vampRefs)) {
                    ++it->second->mPlacableGrenades;
                    break;
                }
            }
        }
    }

    struct NowBlowingGrenade {
        Area area;
        Grenade grenade;
    };

    std::vector<NowBlowingGrenade> blowingGrenades;
    for (Grenade& grenade : mState.mGrenades) {
        for (const auto& area : areas) {
            if (area.mArea.find(grenade.mX, grenade.mY)) {
                blowingGrenades.push_back({ GetBlowArea(grenade), grenade });
                grenade.mTick = -2;
            }
        }
    }

    mState.mGrenades.erase(
        std::remove_if(mState.mGrenades.begin(), mState.mGrenades.end(), [](const auto& grenade) { return grenade.mTick == -2; }), mState.mGrenades.end());

    std::vector<BatSquad> survivorBats;
    survivorBats.reserve(mState.mAllBats.size());

    for (const auto& bat : mState.mAllBats) {
        int injured = 0;

        for (const auto& area : areas) {
            if (area.mArea.find(bat.mX, bat.mY)) {
                injured++;
                std::unordered_set<int> vampiresDamaging;
                for (const auto& element : blowingGrenades) {
                    if (area.mArea.find(element.grenade.mX, element.grenade.mY) && element.area.find(bat.mX, bat.mY)) {
                        vampiresDamaging.insert(element.grenade.mId);
                    }
                }
                for (const auto& vId : vampiresDamaging) {
                    mNewPoints[vId] += 12.F / static_cast<float>(vampiresDamaging.size());
                }
            }
        }

        if (bat.mDensity > injured) {
            survivorBats.emplace_back(bat).mDensity -= injured;
        } else {
            mReachableArea.insert(bat.mX, bat.mY);
        }
    }
    mState.mAllBats = survivorBats;
    mState.mBat1.clear();
    mState.mBat2.clear();
    mState.mBat3.clear();
    for (const auto& bat : mState.mAllBats) {
        if (bat.mDensity == 1) {
            mState.mBat1.push_back(bat);
        } else if (bat.mDensity == 2) {
            mState.mBat2.push_back(bat);
        } else if (bat.mDensity == 3) {
            mState.mBat3.push_back(bat);
        } else {
            CHECK(false, "Some calculation is wrong...");
        }
    }

    std::vector<Vampire> survivorVampires;

    std::vector<Vampire*> vampires = { &mState.mMe };
    for (auto& vampire : mState.mEnemyVampires) {
        vampires.push_back(&vampire);
    }

    for (const auto& vampirePtr : vampires) {
        auto& vampire = *vampirePtr;

        if (vampire.mGhostModeTick == 0 && vampire.mHealth > 0) {

            bool isDead = false;
            bool alreadyDamaged = false;
            std::unordered_set<int> vampiresDamaging;

            for (const auto& area : areas) {
                if (area.mArea.find(vampire.mX, vampire.mY)) {
                    isDead = vampire.mHealth == 1;

                    if (!alreadyDamaged && vampire.mHealth >= 2 && vampire.mGhostModeTick == 0) {
                        if (vampire.mId != mState.mMe.mId) {
                            auto& v = survivorVampires.emplace_back(vampire);
                            v.mHealth--;
                            v.mGhostModeTick = 3;
                        } else {
                            mState.mMe.mHealth--;
                            mState.mMe.mGhostModeTick = 3;
                        }
                        alreadyDamaged = true;
                    }

                    for (const auto& element : blowingGrenades) {
                        if (area.mArea.find(element.grenade.mX, element.grenade.mY) && element.area.find(vampire.mX, vampire.mY)) {
                            vampiresDamaging.insert(element.grenade.mId);
                        }
                    }
                }
            }

            if ((areas.empty() || vampiresDamaging.empty()) && vampire.mId != mState.mMe.mId) {
                survivorVampires.emplace_back(vampire);
            }

            if (isDead) {
                vampire.mHealth = 0;
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
            if (vampire.mId != mState.mMe.mId) {
                survivorVampires.push_back(vampire);
            }
        }
    }
    mState.mEnemyVampires = survivorVampires;
}

void Simulator::HitLight()
{
    if (mState.mMe.mId != -1 && mState.mMe.mGhostModeTick != 3) {
        if (mLitArea.find(mState.mMe.mX, mState.mMe.mY)) {
            mState.mMe.mHealth--;
        }
    }

    for (auto& vampire : mState.mEnemyVampires) {
        if (vampire.mGhostModeTick != 3 && mLitArea.find(vampire.mX, vampire.mY)) {
            vampire.mHealth--;
        }
    }

    mState.mEnemyVampires.erase(
        std::remove_if(mState.mEnemyVampires.begin(), mState.mEnemyVampires.end(), [](const Vampire& vampire) { return vampire.mHealth < 1; }),
        mState.mEnemyVampires.end());
}

void Simulator::PlantGrenades()
{
    std::vector<Vampire*> vampRefs;
    vampRefs.push_back(&mState.mMe);
    for (auto& element : mState.mEnemyVampires) {
        vampRefs.push_back(&element);
    }

    for (const auto& vampire : vampRefs) {
        if (vampire->mHealth <= 0) {
            continue;
        }
        if (const auto it = mVampireMoves.find(vampire->mId); it != mVampireMoves.end()) {
            if (it->second.mPlaceGrenade) {
                if (vampire->mGhostModeTick != 0) {
                    continue;
                }
                if (vampire->mPlacableGrenades > 0) {
                    mState.mGrenades.push_back({ vampire->mId, vampire->mX, vampire->mY, 5, vampire->mGrenadeRange });
                    --vampire->mPlacableGrenades;
                    mReachableArea.clear(vampire->mX, vampire->mY);
                }
            }
        }
    }
}

void Simulator::ThrowGrenades()
{
    std::vector<Vampire*> vampRefs;
    vampRefs.push_back(&mState.mMe);
    for (auto& element : mState.mEnemyVampires) {
        vampRefs.push_back(&element);
    }

    for (const auto& vampire : vampRefs) {
        if (vampire->mHealth <= 0) {
            continue;
        }
        if (const auto it = mVampireMoves.find(vampire->mId); it != mVampireMoves.end()) {
            if (it->second.mThrow) {
                // ???
                // if (vampire->mGhostModeTick != 0) {
                //     continue;
                // }
                const auto& th = *(it->second.mThrow);
                if (th.mDistance > mGameDescription.mGrenadeRadius + 1) {
                    continue;
                }
                std::pair<int, int> origin;
                std::pair<int, int> target;
                switch (th.mDirection) {
                case Throw::Direction::Up:
                    origin = std::make_pair(vampire->mX, vampire->mY - 1);
                    target = std::make_pair(vampire->mX, vampire->mY - 1 - th.mDistance);
                    break;
                case Throw::Direction::Down:
                    origin = std::make_pair(vampire->mX, vampire->mY + 1);
                    target = std::make_pair(vampire->mX, vampire->mY + 1 + th.mDistance);
                    break;
                case Throw::Direction::Left:
                    origin = std::make_pair(vampire->mX - 1, vampire->mY);
                    target = std::make_pair(vampire->mX - 1 - th.mDistance, vampire->mY);
                    break;
                case Throw::Direction::Right:
                    origin = std::make_pair(vampire->mX + 1, vampire->mY);
                    target = std::make_pair(vampire->mX + 1 + th.mDistance, vampire->mY);
                    break;
                case Throw::Direction::XUp:
                    origin = std::make_pair(vampire->mX, vampire->mY);
                    target = std::make_pair(vampire->mX, vampire->mY - th.mDistance);
                    break;
                case Throw::Direction::XDown:
                    origin = std::make_pair(vampire->mX, vampire->mY);
                    target = std::make_pair(vampire->mX, vampire->mY + th.mDistance);
                    break;
                case Throw::Direction::XLeft:
                    origin = std::make_pair(vampire->mX, vampire->mY);
                    target = std::make_pair(vampire->mX - th.mDistance, vampire->mY);
                    break;
                case Throw::Direction::XRight:
                    origin = std::make_pair(vampire->mX, vampire->mY);
                    target = std::make_pair(vampire->mX + th.mDistance, vampire->mY);
                    break;
                }

                if (target.first < 0 || target.first > mGameDescription.mMapSize - 1 || target.second < 0 || target.second > mGameDescription.mMapSize - 1) {
                    continue;
                }

                if (!mThrowableArea.find(target.first, target.second)) {
                    continue;
                }

                bool originHasNonMeBomb = false;
                bool successfullThrow = false;
                for (auto& grenade : mState.mGrenades) {
                    if (grenade.mX == origin.first && grenade.mY == origin.second) {
                        if (grenade.mId == mState.mMe.mId) {
                            grenade.mX = target.first;
                            grenade.mY = target.second;
                            successfullThrow = true;
                        } else {
                            originHasNonMeBomb = true;
                        }
                    }
                }

                if (successfullThrow) {
                    mReachableArea.clear(target.first, target.second);
                }
                if (!originHasNonMeBomb) {
                    mReachableArea.insert(origin.first, origin.second);
                }
            }
        }
    }
}

void Simulator::Move()
{
    std::vector<Vampire*> vampRefs;
    vampRefs.push_back(&mState.mMe);
    for (auto& element : mState.mEnemyVampires) {
        vampRefs.push_back(&element);
    }

    for (const auto& vampire : vampRefs) {
        if (vampire->mHealth <= 0) {
            continue;
        }
        if (const auto it = mVampireMoves.find(vampire->mId); it != mVampireMoves.end()) {
            for (const auto& d : it->second.mSteps) {
                size_t newX = static_cast<size_t>(vampire->mX);
                size_t newY = static_cast<size_t>(vampire->mY);
                switch (d) {
                case 'U':
                    --newY;
                    break;
                case 'R':
                    ++newX;
                    break;
                case 'D':
                    ++newY;
                    break;
                case 'L':
                    --newX;
                    break;
                default:
                    CHECK(false, "Invalid direction!");
                }
                if (!mReachableArea.find(static_cast<int>(newX), static_cast<int>(newY))) {
                    break;
                }
                vampire->mX = static_cast<int>(newX);
                vampire->mY = static_cast<int>(newY);
            }
        }
    }
}

bool Simulator::IsValidMove(int id, const Answer& move) const
{
    return IsValidMove(id, ActionSequence(move));
}

bool Simulator::IsValidMove(int id, const ActionSequence& move) const
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
        CHECK(false, "Invalid id: " + std::to_string(id));
    }

    if (move.IsGrenade()) {
        if (vampire->mGhostModeTick != 0) {
            return false;
        }
        if (vampire->mPlacableGrenades < 1) {
            return false;
        }
    }

    const auto numberOfSteps = move.GetNumberOfSteps();
    if (numberOfSteps > 2 && vampire->mRunningShoesTick == 0) {
        return false;
    }

    if (numberOfSteps > 3) {
        return false;
    }

    int x = vampire->mX;
    int y = vampire->mY;
    for (int n = 0; n < numberOfSteps; ++n) {
        const auto d = move.GetNthStep(n);

        switch (d) {
        case 0:
            y--;
            break;
        case 1:
            y++;
            break;
        case 2:
            x--;
            break;
        case 3:
            x++;
            break;
        default:
            throw std::runtime_error("Invalid step");
        }

        if (!mReachableArea.find(x, y)) {
            return false;
        }
    }

    return true;
}

std::vector<Simulator::BlowArea> Simulator::GetBlowAreas(const bool blowNow /*= false*/)
{
    std::vector<Simulator::BlowArea> retVal;
    std::unordered_map<std::pair<int, int>, std::vector<std::pair<const Grenade*, bool>>, boost::hash<std::pair<int, int>>> grenadesByPos;
    // collect grenades: [x, y] -> vector<{grenade*, bAlreadyProcessed}>
    for (const auto& grenade : mState.mGrenades) {
        grenadesByPos[{ grenade.mX, grenade.mY }].emplace_back(&grenade, false);
    }
    // go over on all grenade positions
    for (auto& [_, grenadeDescVec] : grenadesByPos) {
        // go over on grenades on the given position
        for (std::pair<const Grenade*, bool>& grenadeDesc : grenadeDescVec) {
            if (blowNow && grenadeDesc.first->mTick != 0) {
                continue;
            }

            // skip if the grenade was already processed
            if (grenadeDesc.second) {
                continue;
            }

            Simulator::BlowArea ba(mGameDescription.mMapSize);
            // queue for grenades that are in a given area
            std::vector<std::pair<const Grenade*, bool>*> grenadesToProcess;
            grenadesToProcess.push_back(&grenadeDesc);
            // while the queue is not empty
            while (!grenadesToProcess.empty()) {
                std::pair<const Grenade*, bool>& grenade = *grenadesToProcess.back();
                grenadesToProcess.pop_back();

                // set the given grenade to 'processed'
                grenade.second = true;
                // get the are of the grenade
                Simulator::Area area = GetBlowArea(*(grenade.first));
                for (const auto& position : area.getAsVector()) {
                    // extend the whole blow are (chain) with the given grenade's area
                    ba.mArea.insert(position.first, position.second);
                    // check if the area's part contains an other grenade
                    if (auto gv = grenadesByPos.find(position); gv != grenadesByPos.end()) {
                        // if so, iterate on the grenades of the given position
                        for (std::pair<const Grenade*, bool>& g : gv->second) {
                            // if it's not marked already processed, add it to the queue
                            if (!g.second) {
                                grenadesToProcess.push_back(&g);
                            }
                        }
                    }
                }
            }

            for (const auto& g : mState.mGrenades) {
                if (ba.mArea.find(g.mX, g.mY)) {
                    ba.mTickCount = ba.mTickCount == -1 ? g.mTick : std::min(ba.mTickCount, g.mTick);
                    ba.mVampireIds.insert(g.mId);
                }
            }

            retVal.push_back(ba);
        }
    }
    return retVal;
}

Simulator::Area Simulator::GetBlowArea(const Grenade& grenade)
{
    Area area(mGameDescription.mMapSize);
    const auto checkExplosion = [&mState = mState, &mGameDescription = mGameDescription, &area](const int px, const int py) {
        if (px == 0 || py == 0 || px == mGameDescription.mMapSize - 1 || py == mGameDescription.mMapSize - 1 || (!(px % 2) && !(py % 2))) {
            return false;
        }
        area.insert(px, py);
        for (const auto& bat : mState.mAllBats) {
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
