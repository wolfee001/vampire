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
#ifdef GAME_WITH_FRAMEWORK
    Framework::GetInstance().SetGameDescription(mGameDescription, startInfos);
#endif

    mMagic = std::make_unique<UsualMagic>(mGameDescription);
}

std::vector<std::string> solver::processTick(const std::vector<std::string>& infos)
{
    auto tick = parseTickDescription(infos);
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
    mTickDescription = tick;

#ifdef GAME_WITH_FRAMEWORK
    Framework::GetInstance().Update(mTickDescription, infos);
#endif

    Answer answer = mMagic->Tick(mTickDescription);

    std::vector<std::string> commands { "RES " + std::to_string(tick.mRequest.mGameId) + " " + std::to_string(tick.mRequest.mTick) + " "
        + std::to_string(tick.mRequest.mVampireId) };
    if (answer.mPlaceGrenade) {
        commands.emplace_back("GRENADE");
    }
    if (!answer.mSteps.empty()) {
        std::string moveCommand = "MOVE";
        for (const auto& step : answer.mSteps) {
            moveCommand += std::string(" ") + step;
        }
        commands.emplace_back(moveCommand);
    }

    return commands;
}
