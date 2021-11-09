#pragma once

#include "i_magic.h"

#include "gabor_magic.h"
#include "usual_magic.h"

class FinalMagic : public IMagic {
public:
    explicit FinalMagic(const GameDescription& gameDescription);

    Answer Tick(const TickDescription& tickDescription, const Simulator::NewPoints& points);

    UsualMagic mUsualMagic;
    GaborMagic mGaborMagic;
};
