#include "final_magic.h"
#include "models.h"
#include <chrono>

FinalMagic::FinalMagic(const GameDescription& gameDescription)
    : IMagic(gameDescription)
    , mUsualMagic(gameDescription)
    , mGaborMagic(gameDescription)
{
    // Maybe some constructor magic? :)
}

Answer FinalMagic::Tick(const TickDescription& tickDescription, const std::map<int, float>& points)
{
    std::chrono::milliseconds start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
    mUsualMagic.SetTickTimeout(mTimeout / 4);
    mUsualMagic.Tick(tickDescription, points);
    mGaborMagic.SetPhase(mUsualMagic.mPhase);
    mGaborMagic.SetAvoids(mUsualMagic.mAvoids);
    /*
        if (mUsualMagic.mPhase == PHASE1) {
            mGaborMagic.SetBombSequence(mUsualMagic.mPath);
        } else if (mUsualMagic.mPhase == ITEM || mUsualMagic.mPhase == BETWEEN_ITEMS) {
            mGaborMagic.SetPathSequence(mUsualMagic.mPath);
        }
    */
    if (mUsualMagic.mPhase == ITEM) {
        mGaborMagic.SetPathSequence(mUsualMagic.mPath);
    } else {
        mGaborMagic.SetPhase(NONE);
    }

    std::chrono::milliseconds usualTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()) - start;
    mGaborMagic.SetTickTimeout(mTimeout - usualTime);
    const auto& retVal = mGaborMagic.Tick(tickDescription, points);
    std::chrono::milliseconds totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()) - start;

    std::cerr << "Total allowed time: " << mTimeout.count() << " ms" << std::endl;
    std::cerr << "Usual magic time: " << usualTime.count() << " ms" << std::endl;
    std::cerr << "Gabor magic time: " << totalTime.count() << " ms" << std::endl;

    return retVal;
}
