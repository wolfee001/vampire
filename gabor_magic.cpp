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

Answer GaborMagic::Tick(const TickDescription& tickDescription, const std::map<int, float>& points)
{
    Search search(tickDescription, mGameDescription, tickDescription.mMe.mId);

    const auto t1 = std::chrono::steady_clock::now();

    for (size_t i = 0; i < 10; ++i) {
        if (!search.CalculateNextLevel(t1 + std::chrono::milliseconds(500))) {
            std::cerr << "Calculation timeout at level " << i << std::endl;
            break;
        }
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