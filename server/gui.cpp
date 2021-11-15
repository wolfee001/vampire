#include "gui.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <ctime>
#include <fmt/core.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#include <iostream>
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

#include "server.h"

#include "../check.h"
#include "../simulator.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GUI& GUI::GetInstance()
{
    static GUI instance;
    return instance;
}

void GUI::SetGameDescription(const GameDescription& description)
{
    mRenderLock.lock();
    mGameDescription = description;
    mRenderLock.unlock();
}

void GUI::SetVampireName(int id, const std::string& name)
{
    mVampireNames[id] = name;
}

void GUI::Update(const TickDescription& description, const std::map<int, float>& points)
{
    mRenderLock.lock();
    mTickDescription = description;
    mCumulatedPoints = points;
    mRenderLock.unlock();
}

void GUI::Run()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return;
    }
    GLFWwindow* window = glfwCreateWindow(1076, 817, "SZERVER, BASZOD!", NULL, NULL);
    if (window == nullptr) {
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

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

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    srand(time(nullptr));

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool gameIsRunning = false;
    int mapSelector = 0;
    int playerCount = 3;
    int seed = rand() % 1000;

    // Main loop
    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        mRenderLock.lock();

        {
            ImGui::SetNextWindowPos({ 0, 0 });
            ImGui::SetNextWindowSize({ 278, 140 });
            ImGui::Begin("Server", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Text(gameIsRunning ? "GAME IS RUNNING!" : "READY TO PLAY");
            ImGui::BeginDisabled(gameIsRunning);
            ImGui::SetNextItemWidth(170.F);
            ImGui::Combo("Select map", &mapSelector, " 1\0 2\0 3\0 4\0 5\0 6\0 7\0 8\0 9\0 10\0 EMPTY\0");
            ImGui::SetNextItemWidth(170.F);
            ImGui::Combo("Player", &playerCount, " 1\0 2\0 3\0 4\0");
            ImGui::SetNextItemWidth(170.F);
            ImGui::InputInt("Seed", &seed);
            ImGui::SameLine();
            if (ImGui::Button("RANDOM")) {
                seed = rand() % 1000;
            }
            if (ImGui::Button("START")) {
                mGameDescription = {};
                mTickDescription = {};
                mCumulatedPoints = {};
                mVampireNames = {};
                std::thread t([&gameIsRunning, &mapSelector, &playerCount, &seed]() {
                    gameIsRunning = true;
                    Levels levels;
                    RunGame(playerCount + 1, levels.mLevels[mapSelector], seed);
                    gameIsRunning = false;
                    seed = rand() % 1000;
                });
                t.detach();
            }
            ImGui::EndDisabled();
            ImGui::End();
        }

        {
            std::vector<Vampire> vamps = { mTickDescription.mMe };
            vamps[0].mId = 1;
            for (int i = 2; i < 5; ++i) {
                auto it
                    = std::find_if(mTickDescription.mEnemyVampires.begin(), mTickDescription.mEnemyVampires.end(), [i](const auto& v) { return v.mId == i; });
                if (it == mTickDescription.mEnemyVampires.end()) {
                    vamps.push_back({});
                    vamps.back().mId = i;
                } else {
                    vamps.push_back(*it);
                }
            }
            ImGui::SetNextWindowPos({ 0, 140 });
            ImGui::SetNextWindowSize({ 278, 455 });
            ImGui::Begin("Vampires###ServerVampires", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

            for (const auto& vampire : vamps) {
                ImGui::BeginGroup();
                ImGui::Text("Version: %s", mVampireNames[vampire.mId].c_str());
                ImGui::BeginGroup();
                ImGui::Image(mAssets["vampire" + std::to_string(vampire.mId)], ImVec2(64, 64));
                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::Text("Health:");
                ImGui::Text("Placable grenades:");
                ImGui::Text("Grenade range:");
                ImGui::Text("Running shoes:");
                ImGui::Text("Points:");
                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::Text("%d", vampire.mHealth);
                ImGui::Text("%d", vampire.mPlacableGrenades);
                ImGui::Text("%d", vampire.mGrenadeRange);
                ImGui::Text("%d", vampire.mRunningShoesTick);
                ImGui::Text("%f", mCumulatedPoints[vampire.mId]);
                ImGui::EndGroup();
                ImGui::EndGroup();
                ImGui::Separator();
            }

            ImGui::End();
        }

        {
            ImGui::SetNextWindowPos({ 278, 0 });
            ImGui::SetNextWindowSize({ 798, 817 });
            if (gameIsRunning) {
                if (mTickDescription.mRequest.mTick == -1) {
                    ImGui::Begin(
                        "Map - WAITING FOR PLAYERS...###ServerMap", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
                } else {
                    ImGui::Begin(fmt::format("Map - TICK {} MAXTICK: {}###ServerMap", mTickDescription.mRequest.mTick, mGameDescription.mMaxTick).c_str(),
                        nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
                }
            } else {
                ImGui::Begin("Map###ServerMap", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            }

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            const ImVec2 p = ImGui::GetCursorScreenPos();
            draw_list->AddRectFilled(p, ImVec2(p.x + 23 * 34, p.y + 23 * 34), IM_COL32(238, 238, 238, 255));

            Simulator simulator(mGameDescription);
            simulator.SetState(mTickDescription);
            const std::vector<Simulator::BlowArea>& blowAreas = simulator.GetBlowAreas();
            const Simulator::Area& litAreas = simulator.GetLitArea();

            for (int x = 0; x < mGameDescription.mMapSize; ++x) {
                for (int y = 0; y < mGameDescription.mMapSize; ++y) {
                    ImVec2 tl = ImGui::GetCursorScreenPos();
                    tl.x += static_cast<float>(x) * 34.f + 1.f;
                    tl.y += static_cast<float>(y) * 34.f + 1.f;
                    if (litAreas.find(x, y)) {
                        draw_list->AddRectFilled(tl, ImVec2(tl.x + 32, tl.y + 32), IM_COL32(255, 255, 101, 255));
                    } else {
                        draw_list->AddRectFilled(tl, ImVec2(tl.x + 32, tl.y + 32), IM_COL32(96, 163, 98, 255));
                    }
                    if (x == 0 || y == 0 || x == mGameDescription.mMapSize - 1 || y == mGameDescription.mMapSize - 1 || (!(x % 2) && !(y % 2))) {
                        draw_list->AddImage(mAssets["bush"], tl, ImVec2(tl.x + 32, tl.y + 32));
                    }
                }
            }

            for (const auto& area : blowAreas) {
                for (const auto& position : area.mArea.getAsVector()) {
                    ImVec2 tl = ImGui::GetCursorScreenPos();
                    tl.x += static_cast<float>(position.first) * 34.f + 1.f;
                    tl.y += static_cast<float>(position.second) * 34.f + 1.f;
                    draw_list->AddRectFilled(tl, ImVec2(tl.x + 32, tl.y + 32), IM_COL32(255, 0, 127, 255));
                }
            }

            for (const auto& vampire : mTickDescription.mEnemyVampires) {
                ImVec2 vampirePos = ImVec2(p.x + static_cast<float>(vampire.mX) * 34 + 1, p.y + static_cast<float>(vampire.mY) * 34 + 1);
                draw_list->AddImage(mAssets["vampire" + std::to_string(vampire.mId)], vampirePos, ImVec2(vampirePos.x + 32, vampirePos.y + 32));
            }

            {
                ImVec2 myPos = ImVec2(p.x + static_cast<float>(mTickDescription.mMe.mX) * 34 + 1, p.y + static_cast<float>(mTickDescription.mMe.mY) * 34 + 1);
                draw_list->AddImage(mAssets["vampire1"], myPos, ImVec2(myPos.x + 32, myPos.y + 32));
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
                    }
                    CHECK(false, "Unhandled type");
                }(pu.mType);
                ImVec2 pos = ImVec2(p.x + static_cast<float>(pu.mX) * 34 + 1, p.y + static_cast<float>(pu.mY) * 34 + 1);
                draw_list->AddImage(mAssets[icon], pos, ImVec2(pos.x + 32, pos.y + 32), { 0, 0 }, { 1, 1 }, IM_COL32(255, 255, 255, 128));
                draw_list->AddText({ pos.x + 12, pos.y + 10 }, IM_COL32(0, 0, 0, 255), fmt::format("{}", pu.mRemainingTick).c_str());
            }

            for (const auto& bat : mTickDescription.mAllBats) {
                std::string batAvatar = "bat" + std::to_string(bat.mDensity);
                ImVec2 pos = ImVec2(p.x + static_cast<float>(bat.mX) * 34 + 1, p.y + static_cast<float>(bat.mY) * 34 + 1);
                draw_list->AddImage(mAssets[batAvatar], pos, ImVec2(pos.x + 32, pos.y + 32));
            }

            ImGui::End();
        }

        mRenderLock.unlock();

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return;
}

void* GUI::LoadAsset(const std::filesystem::path& path)
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