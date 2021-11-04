#pragma once

#include "i_magic.h"

class FinalMagic : public IMagic {
public:
    explicit FinalMagic(const GameDescription& gameDescription);

    Answer Tick(const TickDescription& tickDescription, const std::map<int, float>& points);
};
