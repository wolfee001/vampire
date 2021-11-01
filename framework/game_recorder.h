#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <vector>

class GameRecorder {
public:
    void StartRecording(const std::vector<std::string>& startInfos);
    void AddTick(const std::vector<std::string>& infos);
    void Step(const std::vector<std::string>& infos);

private:
    std::unique_ptr<std::ofstream> mOutput;
};