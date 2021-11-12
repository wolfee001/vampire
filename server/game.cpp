#include "game.h"

#include "../parser.h"
#include "gui.h"
#include <algorithm>

Game::Game(const GameDescription& gd, const TickDescription& zeroTick, int playerCount)
    : mGameDescription(gd)
    , mSimulator(mGameDescription)
{
    auto tick = zeroTick;
    tick.mEnemyVampires.erase(
        std::remove_if(tick.mEnemyVampires.begin(), tick.mEnemyVampires.end(), [&playerCount](const auto& vampire) { return vampire.mId > playerCount; }),
        tick.mEnemyVampires.end());
    mSimulator.SetState(tick);

    mNextPowerupTick = mGameDescription.mMapSize * mGameDescription.mMapSize / 3;

    GUI::GetInstance().SetGameDescription(mGameDescription);
    GUI::GetInstance().Update(tick, mCumulatedPoints);
}

void Game::SetVampireMove(int id, const Answer& move)
{
    mSimulator.SetVampireMove(id, move);
}

std::pair<TickDescription, std::vector<std::pair<int, float>>> Game::Tick()
{
    std::pair<TickDescription, Simulator::NewPoints> tickResp = mSimulator.Tick();
    for (const auto& [id, point] : tickResp.second) {
        mCumulatedPoints[id] += point;
    }

    GUI::GetInstance().Update(tickResp.first, mCumulatedPoints);

    std::pair<TickDescription, std::vector<std::pair<int, float>>> retVal = { tickResp.first, {} };

    if (tickResp.first.mMe.mHealth < 1) {
        retVal.second.emplace_back(1, mCumulatedPoints[1]);
    }
    for (int i = 2; i < 5; ++i) {
        if (std::find_if(tickResp.first.mEnemyVampires.begin(), tickResp.first.mEnemyVampires.end(),
                [i](const auto& element) { return element.mId == i && element.mHealth < 1; })
            != tickResp.first.mEnemyVampires.end()) {
            retVal.second.emplace_back(i, mCumulatedPoints[i]);
        }
    }

    GeneratePowerups(tickResp.first);

    mSimulator.SetState(tickResp.first);

    return retVal;
}

void Game::GeneratePowerups(TickDescription& tick)
{
    if (tick.mRequest.mTick > mGameDescription.mMaxTick - 35) {
        return;
    }
    if (mNextPowerupTick > tick.mRequest.mTick) {
        return;
    }
    if (mNextPowerupTick == tick.mRequest.mTick) {
        std::pair<int, int> position = { 1 + rand() % (mGameDescription.mMapSize - 2), 1 + rand() % (mGameDescription.mMapSize - 2) };
        while (!(position.first % 2) && !(position.second % 2)) {
            position = { 1 + rand() % (mGameDescription.mMapSize - 2), 1 + rand() % (mGameDescription.mMapSize - 2) };
        }
        int t = rand() % 4;
        PowerUp::Type type;
        if (t == 0) {
            type = PowerUp::Type::Battery;
        } else if (t == 1) {
            type = PowerUp::Type::Grenade;
        } else if (t == 2) {
            type = PowerUp::Type::Shoe;
        } else if (t == 3) {
            type = PowerUp::Type::Tomato;
        }

        const int pre = 5 + rand() % 6;
        const int duration = 10 + rand() % 11;

        tick.mPowerUps.push_back({ type, -pre, position.first, position.second, duration });
        tick.mPowerUps.push_back({ type, -pre, mGameDescription.mMapSize - position.first - 1, mGameDescription.mMapSize - position.second - 1, duration });
    }
    if (!tick.mPowerUps.empty()) {
        return;
    }
    mNextPowerupTick = tick.mRequest.mTick + 10 + rand() % 11;
}

float Game::GetPoint(int id)
{
    return mCumulatedPoints[id];
}

void Game::KillVampire(int id)
{
    mSimulator.KillVampire(id);
}