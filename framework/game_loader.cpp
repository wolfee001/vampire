#include "game_loader.h"

#include "../check.h"
#include "../parser.h"

#include <exception>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

GameLoader::GameLoader(const std::string& fileName)
{
    std::ifstream file(fileName.c_str());
    if (!file.is_open()) {
        CHECK(false, "Failed to open file!");
    }

    std::vector<std::string> startInfos;
    for (int i = 0; i < 7; ++i) {
        std::string tmp;
        std::getline(file, tmp);
        startInfos.push_back(tmp.substr(2));
    }
    mDescription.mGameDescription = parseGameDescription(startInfos);
    mDescription.mMessage = startInfos;

    std::vector<std::string> allMessages;
    std::string line;

    while (std::getline(file, line)) {
        allMessages.push_back(line);
    }

    std::vector<std::string> info;
    std::vector<std::string> answer;
    for (size_t i = 0; i < allMessages.size(); ++i) {
        if (allMessages[i][0] == '>') {
            if (!answer.empty() || !info.empty() && allMessages[i].find("REQ") != -1) {
                Step step;
                step.mTick = parseTickDescription(info);
                step.mTickMessage = info;
                step.mAnswer = parseAnswer(answer);
                step.mAnswerMessage = answer;
                mSteps.push_back(step);
                info.clear();
                answer.clear();
            }
            info.push_back(allMessages[i].substr(2));
        }
        if (allMessages[i][0] == '<') {
            answer.push_back(allMessages[i].substr(2));
        }
    }

    if (!info.empty() || !answer.empty()) {
        Step step;
        step.mTick = parseTickDescription(info);
        step.mTickMessage = info;
        step.mAnswer = parseAnswer(answer);
        step.mAnswerMessage = answer;
        mSteps.push_back(step);
    }
}

GameLoader::GameDescriptionWithInfo GameLoader::GetDescription()
{
    return mDescription;
}

std::vector<GameLoader::Step> GameLoader::GetStepUntil(size_t frame)
{
    if (frame > mSteps.size()) {
        CHECK(false, "Invalid frame!");
    }

    return { mSteps.begin(), mSteps.begin() + frame };
}

GameLoader::Step GameLoader::GetFrame(size_t frame)
{
    if (frame > mSteps.size()) {
        CHECK(false, "Invalid frame!");
    }

    return mSteps[frame];
}

size_t GameLoader::GetStepCount()
{
    return mSteps.size();
}
