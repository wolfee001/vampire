#include "gabor_magic.h"
#include "mcts.h"
#include <chrono>

GaborMagic::GaborMagic(const GameDescription& gameDescription)
    : IMagic(gameDescription)
{
    // Maybe some constructor magic? :)
}

#include <iostream>
Answer GaborMagic::Tick(const TickDescription& tickDescription)
{
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1800);

    std::vector<int> playerIds = { tickDescription.mMe.mId };
    for (const auto& v : tickDescription.mEnemyVampires) {
        playerIds.push_back(v.mId);
    }

    MonteCarloTreeSearch mcts(tickDescription, mGameDescription, playerIds);

    size_t steps = 0;
    while (std::chrono::steady_clock::now() < deadline) {
        mcts.Step();
        ++steps;
    }

    auto move = mcts.GetBestMove();
    std::cerr << "steps: " << steps << " grenade: " << move.mPlaceGrenade << " moves: ";
    for (const auto& s : move.mSteps) {
        std::cerr << s << ", ";
    }
    std::cerr << std::endl;
    return move;
}