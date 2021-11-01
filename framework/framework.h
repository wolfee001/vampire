#pragma once

#include "../models.h"
#include "../solver.h"
#include <chrono>
#include <filesystem>
#include <imgui.h>
#include <map>
#include <mutex>

#include "game_loader.h"
#include "game_recorder.h"

class Framework {
    struct PlayBook {
        GameLoader mGameLoader;
        bool mSolverIsStateful = false;
        size_t mStep = 0;
        solver mSolver;
        bool mIsCorrupted = false;
        bool mIsPlaying = false;
        std::chrono::milliseconds mLastStepTime = std::chrono::milliseconds(0);
        int mStepSpeed = 200;
    };

public:
    static Framework& GetInstance();

    void SetGameDescription(const GameDescription& description, const std::vector<std::string>& infos);
    void Update(const TickDescription& description, const std::vector<std::string>& infos);
    void Step(const std::vector<std::string>& infos);

    void Render();

private:
    Framework();

    void* LoadAsset(const std::filesystem::path& path);

private:
    std::mutex mRenderLock;
    GameDescription mGameDescription;
    TickDescription mTickDescription;

    int mMapSelector = 0;
    bool mRecordGame = true;

    std::map<std::string, void*> mAssets;

    GameRecorder mGameRecorder;

    std::map<int, std::string> mVampireAvatarMapping;

    PlayBook mPlayBook;
};