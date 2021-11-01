#include "framework.h"
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fmt/core.h>
#include <imgui.h>
#include <stdexcept>
#include <string>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif
#include <GL/gl.h>
#endif

#include <portable-file-dialogs.h>

#include "game_loader.h"
#include "model_wrapper.h"

#include "../parser.h"
#include "../simulator.h"

int __main(int, char**);

/* static */ Framework& Framework::GetInstance()
{
    static Framework instance;
    return instance;
}

void Framework::SetGameDescription(const GameDescription& description, const std::vector<std::string>& infos)
{
    mRenderLock.lock();
    mGameDescription = description;
    if (mRecordGame) {
        mGameRecorder.StartRecording(infos);
    }
    mVampireAvatarMapping.clear();
    mRenderLock.unlock();
}

void Framework::Update(const TickDescription& description, const std::vector<std::string>& infos)
{
    mRenderLock.lock();
    mTickDescription = description;
    if (mVampireAvatarMapping.empty()) {
        mVampireAvatarMapping[mTickDescription.mEnemyVampires[0].mId] = "vampire2";
        mVampireAvatarMapping[mTickDescription.mEnemyVampires[1].mId] = "vampire3";
        mVampireAvatarMapping[mTickDescription.mEnemyVampires[2].mId] = "vampire4";
    }
    if (mRecordGame) {
        mGameRecorder.AddTick(infos);
    }
    mRenderLock.unlock();
}

void Framework::Step(const std::vector<std::string>& infos)
{
    if (mRecordGame) {
        mGameRecorder.Step(infos);
    }
}

void Framework::Render()
{
    mRenderLock.lock();

    ImGui::NewFrame();

    // ImGui::ShowDemoWindow();

    {
        ImGui::Begin("Playback");

        if (ImGui::Button("LOAD")) {
            auto result = pfd::open_file("Open recorded game", PROJECT_DIR "/data").result();
            if (!result.empty()) {
                mPlayBook = PlayBook();
                mTickDescription = TickDescription();
                mGameDescription = GameDescription();
                mPlayBook.mGameLoader = GameLoader(result[0]);
                std::thread t([&mPlayBook = mPlayBook]() { mPlayBook.mSolver.startMessage(mPlayBook.mGameLoader.GetDescription().mMessage); });
                t.detach();
            }
        }

        if (mPlayBook.mGameLoader.GetDescription().mGameDescription.mGameId != -1) {
            ImGui::SameLine();
            if (ImGui::Button("CLEAR")) {
                mPlayBook = PlayBook();
                mTickDescription = TickDescription();
                mGameDescription = GameDescription();
            }

            ImGui::Text("LOADED: %d", mPlayBook.mGameLoader.GetDescription().mGameDescription.mGameId);
            ImGui::SameLine();
            ImGui::Text("STEP: %zu", mPlayBook.mStep);
            ImGui::SameLine();
            ImGui::Text("MAX STEP: %zu", mPlayBook.mGameLoader.GetStepCount() - 1);

            if (mPlayBook.mIsCorrupted) {
                ImGui::Text("THE GAME IS CORRUPTED. GOOD DAY SIR!");
            } else {
                ImGui::Checkbox("Solver is stateful", &mPlayBook.mSolverIsStateful);

                ImGui::BeginDisabled(mPlayBook.mIsPlaying);
                static int jumpTo = 0;
                ImGui::InputInt("Next step", &jumpTo);
                ImGui::SameLine();
                if (ImGui::Button("Jump")) {
                    mPlayBook.mStep = std::max(int(0), std::min(jumpTo, static_cast<int>(mPlayBook.mGameLoader.GetStepCount())));
                    if (mPlayBook.mSolverIsStateful) {
                        std::vector<GameLoader::Step> steps = mPlayBook.mGameLoader.GetStepUntil(mPlayBook.mStep);
                        std::thread t([steps, &mPlayBook = mPlayBook]() {
                            for (const auto& element : steps) {
                                const auto& resp = mPlayBook.mSolver.processTick(element.mTickMessage);
                                if (resp != element.mAnswerMessage) {
                                    mPlayBook.mIsCorrupted = true;
                                    mPlayBook.mIsPlaying = false;
                                    return;
                                }
                            }
                        });
                        t.detach();
                    } else {
                        GameLoader::Step step = mPlayBook.mGameLoader.GetFrame(mPlayBook.mStep);
                        std::thread t([step, &mPlayBook = mPlayBook]() {
                            const auto& resp = mPlayBook.mSolver.processTick(step.mTickMessage);
                            if (resp != step.mAnswerMessage) {
                                mPlayBook.mIsCorrupted = true;
                                return;
                            }
                        });
                        t.detach();
                    }
                }
                ImGui::EndDisabled();

                ImGui::BeginDisabled(mPlayBook.mIsPlaying);
                if (ImGui::Button("|< ")) {
                    if (mPlayBook.mStep > 0) {
                        mPlayBook.mStep--;
                        if (mPlayBook.mSolverIsStateful) {
                            std::vector<GameLoader::Step> steps = mPlayBook.mGameLoader.GetStepUntil(mPlayBook.mStep);
                            std::thread t([steps, &mPlayBook = mPlayBook]() {
                                for (const auto& element : steps) {
                                    const auto& resp = mPlayBook.mSolver.processTick(element.mTickMessage);
                                    if (resp != element.mAnswerMessage) {
                                        mPlayBook.mIsCorrupted = true;
                                        mPlayBook.mIsPlaying = false;
                                        return;
                                    }
                                }
                            });
                            t.detach();
                        } else {
                            GameLoader::Step step = mPlayBook.mGameLoader.GetFrame(mPlayBook.mStep);
                            std::thread t([step, &mPlayBook = mPlayBook]() {
                                const auto& resp = mPlayBook.mSolver.processTick(step.mTickMessage);
                                if (resp != step.mAnswerMessage) {
                                    mPlayBook.mIsCorrupted = true;
                                    return;
                                }
                            });
                            t.detach();
                        }
                    }
                }
                ImGui::EndDisabled();
                ImGui::SameLine();
                if (ImGui::Button(mPlayBook.mIsPlaying ? " || " : " > ")) {
                    mPlayBook.mIsPlaying = !mPlayBook.mIsPlaying;
                }
                ImGui::SameLine();
                ImGui::BeginDisabled(mPlayBook.mIsPlaying);
                if (ImGui::Button(" >|")) {
                    if (mPlayBook.mStep < mPlayBook.mGameLoader.GetStepCount() - 1) {
                        mPlayBook.mStep++;
                        GameLoader::Step step = mPlayBook.mGameLoader.GetFrame(mPlayBook.mStep);
                        std::thread t([step, &mPlayBook = mPlayBook]() {
                            const auto& resp = mPlayBook.mSolver.processTick(step.mTickMessage);
                            if (resp != step.mAnswerMessage) {
                                mPlayBook.mIsCorrupted = true;
                                return;
                            }
                        });
                        t.detach();
                    }
                }
                ImGui::EndDisabled();

                ImGui::SliderInt("Playback speed", &mPlayBook.mStepSpeed, 50, 1000);
            }
        }

        ImGui::End();
    }

    {
        if (mPlayBook.mIsPlaying) {
            std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            std::chrono::milliseconds delta = time - mPlayBook.mLastStepTime;
            if (delta.count() > mPlayBook.mStepSpeed) {
                mPlayBook.mLastStepTime = time;
                if (mPlayBook.mStep < mPlayBook.mGameLoader.GetStepCount() - 1) {
                    mPlayBook.mStep++;
                    GameLoader::Step step = mPlayBook.mGameLoader.GetFrame(mPlayBook.mStep);
                    std::thread t([step, &mPlayBook = mPlayBook]() {
                        const auto& resp = mPlayBook.mSolver.processTick(step.mTickMessage);
                        if (resp != step.mAnswerMessage) {
                            mPlayBook.mIsCorrupted = true;
                            return;
                        }
                    });
                    t.detach();
                }
            }
        }
    }

    {
        ImGui::Begin("Local server");

        if (ImGui::Button("GO", ImVec2(-1.F, 0.F))) {
            std::thread t([&mMapSelector = mMapSelector]() {
                std::string selectedMap = std::to_string(mMapSelector);
                std::string programName = "fake_program_name";
                std::string host = "127.0.0.1";
                std::string port = "6789";
#if defined(__GNUC__) && !defined(__llvm__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
                char* params[4] = { const_cast<char*>(programName.data()), const_cast<char*>(selectedMap.data()), const_cast<char*>(host.data()),
                    const_cast<char*>(port.data()) };
#if defined(__GNUC__) && !defined(__llvm__)
#pragma GCC diagnostic pop
#endif
                __main(4, params);
            });
            t.detach();
        }

        ImGui::End();
    }

    {
        ImGui::Begin("Game handler");

        ImGui::Combo("Select map", &mMapSelector, " RANDOM\0 1\0 2\0 3\0 4\0 5\0 6\0 7\0 8\0 9\0 10\0");
        ImGui::Checkbox("Record game", &mRecordGame);

        if (ImGui::Button("GO", ImVec2(-1.F, 0.F))) {
            std::thread t([&mMapSelector = mMapSelector]() {
                std::string selectedMap = std::to_string(mMapSelector);
                std::string programName = "fake_program_name";
#if defined(__GNUC__) && !defined(__llvm__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
                char* params[2] = { const_cast<char*>(programName.data()), const_cast<char*>(selectedMap.data()) };
#if defined(__GNUC__) && !defined(__llvm__)
#pragma GCC diagnostic pop
#endif
                __main(2, params);
            });
            t.detach();
        }

        ImGui::End();
    }

    {
        ImGui::Begin("Map");

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImVec2 p = ImGui::GetCursorScreenPos();
        draw_list->AddRectFilled(p, ImVec2(p.x + 23 * 34, p.y + 23 * 34), IM_COL32(238, 238, 238, 255));

        for (int x = 0; x < mGameDescription.mMapSize; ++x) {
            for (int y = 0; y < mGameDescription.mMapSize; ++y) {
                ImVec2 tl = ImGui::GetCursorScreenPos();
                tl.x += static_cast<float>(x) * 34.f + 1.f;
                tl.y += static_cast<float>(y) * 34.f + 1.f;
                draw_list->AddRectFilled(tl, ImVec2(tl.x + 32, tl.y + 32), IM_COL32(96, 163, 98, 255));
                if (x == 0 || y == 0 || x == mGameDescription.mMapSize - 1 || y == mGameDescription.mMapSize - 1 || (!(x % 2) && !(y % 2))) {
                    draw_list->AddImage(mAssets["bush"], tl, ImVec2(tl.x + 32, tl.y + 32));
                }
            }
        }

        Simulator simulator(mGameDescription);
        simulator.SetState(mTickDescription);
        std::vector<Simulator::BlowArea> blowAreas = simulator.GetBlowAreas();

        for (const auto& area : blowAreas) {
            for (const auto& position : area.mArea) {
                ImVec2 tl = ImGui::GetCursorScreenPos();
                tl.x += static_cast<float>(position.first) * 34.f + 1.f;
                tl.y += static_cast<float>(position.second) * 34.f + 1.f;
                draw_list->AddRectFilled(tl, ImVec2(tl.x + 32, tl.y + 32), IM_COL32(255, 0, 127, 255));
            }
        }

        {
            ImVec2 myPos = ImVec2(p.x + static_cast<float>(mTickDescription.mMe.mX) * 34 + 1, p.y + static_cast<float>(mTickDescription.mMe.mY) * 34 + 1);
            draw_list->AddImage(mAssets["vampire1"], myPos, ImVec2(myPos.x + 32, myPos.y + 32));
        }

        for (const auto& vampire : mTickDescription.mEnemyVampires) {
            ImVec2 vampirePos = ImVec2(p.x + static_cast<float>(vampire.mX) * 34 + 1, p.y + static_cast<float>(vampire.mY) * 34 + 1);
            draw_list->AddImage(mAssets[mVampireAvatarMapping[vampire.mId]], vampirePos, ImVec2(vampirePos.x + 32, vampirePos.y + 32));
        }

        for (const auto& bat : mTickDescription.mAllBats) {
            std::string batAvatar = "bat" + std::to_string(bat.mDensity);
            ImVec2 pos = ImVec2(p.x + static_cast<float>(bat.mX) * 34 + 1, p.y + static_cast<float>(bat.mY) * 34 + 1);
            draw_list->AddImage(mAssets[batAvatar], pos, ImVec2(pos.x + 32, pos.y + 32));
        }

        for (const auto& grenade : mTickDescription.mGrenades) {
            if (grenade.mTick > 0) {
                ImVec2 pos = ImVec2(p.x + static_cast<float>(grenade.mX) * 34 + 1, p.y + static_cast<float>(grenade.mY) * 34 + 1);
                draw_list->AddImage(mAssets["grenade"], pos, ImVec2(pos.x + 32, pos.y + 32));
                draw_list->AddText({ pos.x + 12, pos.y + 10 }, IM_COL32(255, 255, 255, 255), fmt::format("{}", grenade.mTick).c_str());
            } else {
                const auto drawExplosion = [&mTickDescription = mTickDescription, &mGameDescription = mGameDescription, &mAssets = mAssets, &draw_list, &p](
                                               const int px, const int py) {
                    ImVec2 pos = ImVec2(p.x + static_cast<float>(px) * 34 + 1, p.y + static_cast<float>(py) * 34 + 1);
                    if (px == 0 || py == 0 || px == mGameDescription.mMapSize - 1 || py == mGameDescription.mMapSize - 1 || (!(px % 2) && !(py % 2))) {
                        return false;
                    }
                    draw_list->AddImage(mAssets["explosion"], pos, ImVec2(pos.x + 32, pos.y + 32));
                    for (const auto& bat : mTickDescription.mAllBats) {
                        if (bat.mX == px && bat.mY == py) {
                            return false;
                            ;
                        }
                    }
                    return true;
                };
                for (int x = 0; x < grenade.mRange + 1; ++x) {
                    if (!drawExplosion(grenade.mX + x, grenade.mY)) {
                        break;
                    }
                }
                for (int x = 0; x < grenade.mRange + 1; ++x) {
                    if (!drawExplosion(grenade.mX - x, grenade.mY)) {
                        break;
                    }
                }
                for (int y = 0; y < grenade.mRange + 1; ++y) {
                    if (!drawExplosion(grenade.mX, grenade.mY + y)) {
                        break;
                    }
                }
                for (int y = 0; y < grenade.mRange + 1; ++y) {
                    if (!drawExplosion(grenade.mX, grenade.mY - y)) {
                        break;
                    }
                }
            }

            for (const auto& pu : mTickDescription.mPowerUps) {
                std::string icon = [](const PowerUp::Type type) {
                    switch (type) {
                    case PowerUp::Type::Battery:
                        return "battery";
                    case PowerUp::Type::Grenade:
                        return "grenade_pu";
                    case PowerUp::Type::Shoe:
                        return "shoe";
                    case PowerUp::Type::Tomato:
                        return "tomato";
                    default:
                        throw std::runtime_error("Unhandled type!");
                    }
                }(pu.mType);
                ImVec2 pos = ImVec2(p.x + static_cast<float>(pu.mX) * 34 + 1, p.y + static_cast<float>(pu.mY) * 34 + 1);
                draw_list->AddImage(mAssets[icon], pos, ImVec2(pos.x + 32, pos.y + 32));
                draw_list->AddText({ pos.x + 12, pos.y + 10 }, IM_COL32(0, 0, 0, 255), fmt::format("{}", pu.mRemainingTick).c_str());
            }
        }

        ImGui::End();
    }

    {
        ImGui::Begin("Game parameters");

        GameDescriptionWrapper gameDescription(mGameDescription);
        AddGuiElement(gameDescription);

        ImGui::End();
    }

    {
        ImGui::Begin("Tick parameters");

        TickDescriptionWrapper tickDescription(mTickDescription);
        AddGuiElement(tickDescription);

        ImGui::End();
    }

    {
        ImGui::Begin("Vampires");

        ImGui::BeginGroup();
        ImGui::BeginGroup();
        ImGui::Image(mAssets["vampire1"], ImVec2(64, 64));
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("Health:");
        ImGui::Text("Placable grenades:");
        ImGui::Text("Grenade range:");
        ImGui::Text("Running shoes:");
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("%d", mTickDescription.mMe.mHealth);
        ImGui::Text("%d", mTickDescription.mMe.mPlacableGrenades);
        ImGui::Text("%d", mTickDescription.mMe.mGrenadeRange);
        ImGui::Text("%d", mTickDescription.mMe.mRunningShoesTick);
        ImGui::EndGroup();
        ImGui::EndGroup();

        ImGui::Separator();

        for (const auto& vampire : mTickDescription.mEnemyVampires) {
            ImGui::BeginGroup();
            ImGui::BeginGroup();
            ImGui::Image(mAssets[mVampireAvatarMapping[vampire.mId]], ImVec2(64, 64));
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Text("Health:");
            ImGui::Text("Placable grenades:");
            ImGui::Text("Grenade range:");
            ImGui::Text("Running shoes:");
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Text("%d", vampire.mHealth);
            ImGui::Text("%d", vampire.mPlacableGrenades);
            ImGui::Text("%d", vampire.mGrenadeRange);
            ImGui::Text("%d", vampire.mRunningShoesTick);
            ImGui::EndGroup();
            ImGui::EndGroup();
            ImGui::Separator();
        }

        ImGui::End();
    }

    // Rendering
    ImGui::Render();

    mRenderLock.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

Framework::Framework()
{
    mAssets["bush"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "bush.png");
    mAssets["vampire1"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "vampire1.png");
    mAssets["vampire2"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "vampire2.png");
    mAssets["vampire3"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "vampire3.png");
    mAssets["vampire4"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "vampire4.png");
    mAssets["bat1"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "bat1.png");
    mAssets["bat2"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "bat2.png");
    mAssets["bat3"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "bat3.png");
    mAssets["grenade"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "grenade.png");
    mAssets["explosion"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "explosion.png");
    mAssets["battery"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "battery.png");
    mAssets["grenade_pu"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "grenade_pu.png");
    mAssets["shoe"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "shoe.png");
    mAssets["tomato"] = LoadAsset(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "tomato.png");
}

void* Framework::LoadAsset(const std::filesystem::path& path)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(path.string().c_str(), &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return 0;

    for (int i = 0; i < image_width * image_height * 4; i += 4) {
        if (image_data[i] == 96 && image_data[i + 1] == 163 && image_data[i + 2] == 98) {
            image_data[i + 3] = 0;
        }
    }

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    return reinterpret_cast<void*>(static_cast<intptr_t>(image_texture));
}