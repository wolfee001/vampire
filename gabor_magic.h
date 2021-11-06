#pragma once

#include "i_magic.h"
#include <chrono>
#include <vector>

class GaborMagic : public IMagic {
public:
    explicit GaborMagic(const GameDescription& gameDescription);

    Answer Tick(const TickDescription& tickDescription, const std::map<int, float>& points);

    void SetBombSequence(const std::vector<pos_t>& sequence);
    void SetPathSequence(const std::vector<pos_t>& sequence);
    void SetTickTimeout(std::chrono::milliseconds millis);
    void SetPhase(phase_t phase);
    void SetAvoids(int avoidstay);

private:
    std::chrono::milliseconds mTimeout = std::chrono::milliseconds(1000);
    phase_t mPhase;
    int mAvoids;
    std::vector<pos_t> mBombSequence;
    std::vector<pos_t> mPathSequence;
};
