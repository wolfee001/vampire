#pragma once

#include <string>
#include <vector>

struct Level {
    std::vector<std::string> mGameDescription;
    std::vector<std::string> mZeroTick;
};

struct Levels {
    Levels();
    std::vector<Level> mLevels;
};
