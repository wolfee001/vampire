#include "gabor_magic.h"
#include "search.h"
#include <chrono>
#include <iostream>
#include <thread>

GaborMagic::GaborMagic(const GameDescription& gameDescription)
    : IMagic(gameDescription)
{
    // Maybe some constructor magic? :)
}

void GaborMagic::SetLevelLimit(const size_t maxLevel)
{
    mMaxLevel = maxLevel;
}

Answer GaborMagic::Tick(const TickDescription& tickDescription, const Simulator::NewPoints& points)
{
    Search search(tickDescription, mGameDescription, tickDescription.mMe.mId);

    search.SetBombSequence(mBombSequence);
    search.SetPhase(mPhase);
    search.SetAvoids(mAvoids);
    search.SetPathSequence(mPathSequence);

    const auto t1 = std::chrono::steady_clock::now();
    auto calculationTime = mTimeout;

    if (tickDescription.mAllBats.empty() && tickDescription.mPowerUps.empty() && tickDescription.mGrenades.empty()) {
        calculationTime = mTimeout / 2;
    }

    for (size_t i = 0; i < mMaxLevel; ++i) {
        if (!search.CalculateNextLevel(t1 + calculationTime)) {
            std::cerr << "Calculation timeout at level " << i << std::endl;
            break;
        }
    }
    auto move = search.GetBestMove();

    std::cerr << "Calculation took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t1).count() << " ms"
              << std::endl;

    std::cerr << "Avoids: " << mAvoids << std::endl;
    std::cerr << " grenade: " << move.mPlaceGrenade << " moves: ";
    for (const auto& s : move.mSteps) {
        std::cerr << s << ", ";
    }
    std::cerr << std::endl;

    mBombSequence.clear();
    mPathSequence.clear();

    return move;
}

void GaborMagic::SetBombSequence(const std::vector<pos_t>& sequence)
{
    mBombSequence = sequence;
}

void GaborMagic::SetPathSequence(const std::vector<pos_t>& sequence)
{
    mPathSequence = sequence;
}

void GaborMagic::SetPhase(phase_t phase)
{
    mPhase = phase;
}

void GaborMagic::SetAvoids(int avoids)
{
    mAvoids = avoids;
}
