#pragma once

#include "i_magic.h"
#include <chrono>
#include <vector>

class GaborMagic : public IMagic {
public:
    explicit GaborMagic(const GameDescription& gameDescription);

    Answer Tick(const TickDescription& tickDescription, const Simulator::NewPoints& points);

    void SetBombSequence(const std::vector<pos_t>& sequence);
    void SetPathSequence(const std::vector<pos_t>& sequence);
    void SetPhase(phase_t phase);
    void SetAvoids(int avoidstay);
    void SetLevelLimit(const size_t maxLevel);

private:
    size_t mMaxLevel = 10;
    phase_t mPhase = NONE;
    int mAvoids = 0;
    std::vector<pos_t> mBombSequence;
    std::vector<pos_t> mPathSequence;
};
