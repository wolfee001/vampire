#pragma once

#include "i_magic.h"
#include "models.h"

std::vector<pos_t> GetChainAttackBombSequenceForGaborAndKovi(
    const std::vector<pos_t>& path, int itemTick, const GameDescription& gd, const TickDescription& tickDesc);
