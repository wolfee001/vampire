#include "solver.h"
#include "models.h"
#include "parser.h"
#include "usual_magic.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#ifdef GAME_WITH_FRAMEWORK
#include "framework/framework.h"
#endif

#include "magic_selector.h"

#include "final_magic.h"
#include "gabor_magic.h"
#include "usual_magic.h"

#include "check.h"
#include "timeout.h"

std::vector<std::pair<int, int>> solver::line2d(std::pair<int, int> from, const std::pair<int, int>& to)
{
    std::vector<std::pair<int, int>> result;

    int dirvecx = to.first - from.first;
    int dirvecy = to.second - from.second;
    int diagstepx = std::clamp(dirvecx, -1, 1);
    int diagstepy = std::clamp(dirvecy, -1, 1);
    int shortest = std::abs(dirvecx);
    int longest = std::abs(dirvecy);
    int longstepx = shortest < longest ? 0 : diagstepx;
    int longstepy = shortest < longest ? diagstepy : 0;
    if (longest < shortest)
        std::swap(longest, shortest);

    result.reserve(static_cast<size_t>(longest) + 1);

    int numerator = longest >> 1;
    for (int i = 0; i <= longest; i++) {
        result.emplace_back(from);
        numerator += shortest;
        if (numerator < longest) {
            from.first += longstepx;
            from.second += longstepy;
        } else {
            numerator -= longest;
            from.first += diagstepx;
            from.second += diagstepy;
        }
    }

    return result;
}

void solver::startMessage(const std::vector<std::string>& startInfos)
{
    mGameDescription = parseGameDescription(startInfos);
    mSimulator = std::make_unique<Simulator>(mGameDescription);
#ifdef GAME_WITH_FRAMEWORK
    Framework::GetInstance().SetGameDescription(mGameDescription, startInfos);
#endif

#if defined(USUAL_MAGIC)
    mMagic = std::make_unique<UsualMagic>(mGameDescription);
#elif defined(GABOR_MAGIC)
    mMagic = std::make_unique<GaborMagic>(mGameDescription);
#elif defined(FINAL_MAGIC)
    mMagic = std::make_unique<FinalMagic>(mGameDescription);
#else
#pragma error "No magic defined!"
#endif
    mMagic->SetTickTimeout(std::chrono::milliseconds(TIMEOUT));

    mCumulatedPoints[1] = 0.F;
    mCumulatedPoints[2] = 0.F;
    mCumulatedPoints[3] = 0.F;
    mCumulatedPoints[4] = 0.F;
}

std::vector<std::string> solver::processTick(const std::vector<std::string>& infos)
{
    auto tick = parseTickDescription(infos);

    bool skipCalc = false;
    for (const auto& element : tick.mWarnings) {
        if (element == "Wrong") {
            skipCalc = true;
            std::cerr << "SKIP CALC!!" << std::endl;
        }
    }

    tick.mMe.mGhostModeTick = std::max(0, mTickDescription.mMe.mGhostModeTick - 1);
    for (auto& vampire : tick.mEnemyVampires) {
        for (const auto& element : mTickDescription.mEnemyVampires) {
            if (element.mId == vampire.mId) {
                tick.mMe.mGhostModeTick = std::max(0, element.mGhostModeTick - 1);
            }
        }
    }

    if (mTickDescription.mMe.mHealth > tick.mMe.mHealth) {
        tick.mMe.mGhostModeTick = 3;
    }
    for (auto& vampire : tick.mEnemyVampires) {
        for (const auto& element : mTickDescription.mEnemyVampires) {
            if (element.mId == vampire.mId) {
                if (element.mHealth > vampire.mHealth) {
                    vampire.mGhostModeTick = 3;
                    break;
                }
            }
        }
    }

    Simulator::Area puAreas = Simulator::Area(mGameDescription.mMapSize);
    for (const auto& pu : tick.mPowerUps) {
        if (pu.mRemainingTick > 0) {
            puAreas.insert(pu.mX, pu.mY);
        }
    }
    std::vector<Vampire*> vampRefs;
    if (tick.mMe.mHealth > 0) {
        if (puAreas.find(tick.mMe.mX, tick.mMe.mY)) {
            tick.mMe.mRestCount = mTickDescription.mMe.mRestCount + 1;
        } else {
            tick.mMe.mRestCount = 0;
        }
    }
    for (auto& vampire : tick.mEnemyVampires) {
        for (const auto& element : mTickDescription.mEnemyVampires) {
            if (element.mId == vampire.mId) {
                if (element.mHealth > vampire.mHealth) {
                    if (puAreas.find(vampire.mX, vampire.mY)) {
                        vampire.mRestCount = element.mRestCount + 1;
                    } else {
                        vampire.mRestCount = 0;
                    }
                }
            }
        }
    }

    Simulator::NewPoints points;
    if (mTickDescription.mRequest.mGameId != -1) {
        mSimulator->SetState(mTickDescription);
        points = mSimulator->Tick().second;
    }
    for (const auto& [id, point] : points) {
        mCumulatedPoints[id] += point;
    }
    mTickDescription = tick;

#ifdef GAME_WITH_FRAMEWORK
    Framework::GetInstance().Update(mTickDescription, infos);
#endif

    if (!mTickDescription.mEndMessage.mReason.empty()) {
        return {};
    }

    Answer answer;

    if (!skipCalc) {
        try {
            answer = mMagic->Tick(mTickDescription, mCumulatedPoints);
        } catch (const std::exception& e) {
            std::cerr << "exception " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "exception other" << std::endl;
        }
    }

    auto commands = createAnswer(answer, mTickDescription.mRequest.mGameId, mTickDescription.mRequest.mTick, mTickDescription.mRequest.mVampireId);

#ifdef GAME_WITH_FRAMEWORK
    Framework::GetInstance().Step(commands);
#endif

#ifdef CONSOLE_LOG
    for (const auto& element : commands) {
        std::cout << "< " << element << std::endl;
    }
#endif

    return commands;
}

void solver::SetPoints(const Simulator::NewPoints& points)
{
    mCumulatedPoints = points;
}
