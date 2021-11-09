#pragma once

#include "../models.h"
#include <filesystem>
#include <map>
#include <mutex>
#include <string>

class GUI {
public:
    static GUI& GetInstance();

    void SetGameDescription(const GameDescription& description);
    void Update(const TickDescription& description, const std::map<int, float>& points);

    void Run();

private:
    GUI() = default;

    void* LoadAsset(const std::filesystem::path& path);

private:
    std::mutex mRenderLock;
    GameDescription mGameDescription;
    TickDescription mTickDescription;

    std::map<std::string, void*> mAssets;

    std::map<int, float> mCumulatedPoints;
    std::map<int, std::string> mVampireNames;
};
