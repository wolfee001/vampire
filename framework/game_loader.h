#pragma once

#include <string>

#include "../models.h"

class GameLoader {
public:
    struct GameDescriptionWithInfo {
        GameDescription mGameDescription;
        std::vector<std::string> mMessage;
    };

    struct Step {
        TickDescription mTick;
        std::vector<std::string> mTickMessage;
        Answer mAnswer;
        std::vector<std::string> mAnswerMessage;
    };

public:
    GameLoader() = default;
    explicit GameLoader(const std::string& fileName);

    GameDescriptionWithInfo GetDescription();
    std::vector<Step> GetStepUntil(size_t frame);
    Step GetFrame(size_t frame);
    size_t GetStepCount();

private:
    std::vector<Step> mSteps;
    GameDescriptionWithInfo mDescription;
};