#pragma once

#include <map>
#include <string>

#include "../models.h"
#include "../simulator.h"

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
        Simulator::NewPoints mPoints;
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