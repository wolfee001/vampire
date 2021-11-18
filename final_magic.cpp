#include "final_magic.h"
#include "models.h"
#include <chrono>
#include "wolfee_magic.h"

FinalMagic::FinalMagic(const GameDescription& gameDescription)
    : IMagic(gameDescription)
    , mUsualMagic(gameDescription)
    , mGaborMagic(gameDescription)
{
    // Maybe some constructor magic? :)
}

Answer FinalMagic::Tick(const TickDescription& tickDescription, const Simulator::NewPoints& points)
{
    std::chrono::milliseconds start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
    mUsualMagic.SetTickTimeout(mTimeout / 4);
    const auto& retVal1 = mUsualMagic.Tick(tickDescription, points);
    std::chrono::milliseconds usualTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()) - start;
    std::cerr << "Total allowed time: " << mTimeout.count() << " ms" << std::endl;
    std::cerr << "Usual magic time: " << usualTime.count() << " ms" << std::endl;
    if (!retVal1.mSteps.empty()) {
        std::cerr << "Gabor magic skipped!" << std::endl;
        return retVal1;
    }
    mGaborMagic.SetPhase(mUsualMagic.mPhase);
    mGaborMagic.SetAvoids(mUsualMagic.mAvoids);
    mGaborMagic.SetPreferGrenade(mUsualMagic.mPreferGrenade);
    mGaborMagic.SetReachDiff(mUsualMagic.mReachDiff);

#if 0
    std::vector<pos_t> bombseq;
    if (mUsualMagic.mPhase == ITEM && !mUsualMagic.mPath.empty() && mUsualMagic.mReachDiff < 0 && !tickDescription.mPowerUps.empty() && !mUsualMagic.mInPhase1) {
        std::vector<pos_t> comboseq;
        comboseq.push_back(pos_t(tickDescription.mMe.mY, tickDescription.mMe.mX));
        comboseq.insert(comboseq.end(), mUsualMagic.mPath.begin(), mUsualMagic.mPath.end());
        bombseq = GetChainAttackBombSequenceForGaborAndKovi(comboseq, tickDescription.mPowerUps[0].mRemainingTick, mGameDescription, tickDescription);
        if (!bombseq.empty()) {
            std::cerr << "Wolfee override" << std::endl;
            mUsualMagic.mPhase = WOLFEE;
        }
    }

    if (mUsualMagic.mPhase == WOLFEE) {
        mGaborMagic.SetPhase(WOLFEE);
        mGaborMagic.SetBombSequence(bombseq);
        mGaborMagic.SetPathSequence(mUsualMagic.mPath);
    } else 
#endif
    if (mUsualMagic.mPhase == PHASE1) {
        mGaborMagic.SetBombSequence(mUsualMagic.mPath);
    } else if (mUsualMagic.mPhase == ITEM || mUsualMagic.mPhase == BETWEEN_ITEMS || mUsualMagic.mPhase == CHARGE) {
        mGaborMagic.SetPathSequence(mUsualMagic.mPath);
    } else {
        mGaborMagic.SetPhase(NONE);
    }

    mGaborMagic.SetTickTimeout(mTimeout - usualTime);
    const auto& retVal = mGaborMagic.Tick(tickDescription, points);
    std::chrono::milliseconds totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()) - start;
    std::cerr << "Total magic time: " << totalTime.count() << " ms" << std::endl;
    return retVal;
}
