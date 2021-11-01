#include "game_recorder.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

void GameRecorder::StartRecording(const std::vector<std::string>& startInfos)
{
    if (mOutput) {
        throw std::runtime_error("A recording is already running. Something is fucked up.");
    }

    int gameId = 0;
    for (const auto& info : startInfos) {
        std::stringstream stream(info);
        std::string message;
        stream >> message;
        if (message == "GAMEID") {
            stream >> gameId;
            break;
        }
    }

    mOutput.reset(new std::ofstream(std::filesystem::path(PROJECT_DIR) / "data" / std::to_string(gameId)));
    for (const auto& info : startInfos) {
        *mOutput << "> " << info << std::endl;
    }
    mOutput->flush();
}

void GameRecorder::AddTick(const std::vector<std::string>& infos)
{
    for (const auto& info : infos) {
        *mOutput << "> " << info << std::endl;
    }
    mOutput->flush();

    for (const auto& info : infos) {
        std::stringstream stream(info);
        std::string message;
        stream >> message;
        if (message == "END") {
            mOutput->flush();
            mOutput.reset(nullptr);
        }
    }
}

void GameRecorder::Step(const std::vector<std::string>& infos)
{
    for (const auto& info : infos) {
        *mOutput << "< " << info << std::endl;
    }
    mOutput->flush();
}
