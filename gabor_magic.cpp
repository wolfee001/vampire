#include "gabor_magic.h"
#include "search.h"
#include <chrono>
#include <iostream>

GaborMagic::GaborMagic(const GameDescription& gameDescription)
    : IMagic(gameDescription)
{
    // Maybe some constructor magic? :)
}

Answer GaborMagic::Tick(const TickDescription& tickDescription)
{
    Search search(tickDescription, mGameDescription, tickDescription.mMe.mId);

    const auto t1 = std::chrono::steady_clock::now();

    for (size_t i = 0; i < 10; ++i) {
        search.CalculateNextLevel(t1 + std::chrono::milliseconds(1800));
    }
    auto move = search.GetBestMove();

    std::cerr << "Calculation took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t1).count() << " ms"
              << std::endl;

    std::cerr << " grenade: " << move.mPlaceGrenade << " moves: ";
    for (const auto& s : move.mSteps) {
        std::cerr << s << ", ";
    }
    std::cerr << std::endl;
    return move;
}