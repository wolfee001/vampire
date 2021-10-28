#include "framework.h"
#include <chrono>
#include <filesystem>
#include <imgui.h>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

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
    mGameRecorder.StartRecording(infos);
    mRenderLock.unlock();
}

void Framework::Update(const TickDescription& description, const std::vector<std::string>& infos)
{
    mRenderLock.lock();
    mTickDescription = description;
    mGameRecorder.AddTick(infos);
    mRenderLock.unlock();
}

void Framework::Render()
{
    mRenderLock.lock();

    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    {
        ImGui::Begin("Game handler");

        ImGui::Combo("Select map", &mMapSelector, " RANDOM\0 1\0 2\0 3\0 4\0 5\0 6\0 7\0 8\0 9\0 10\0");
        ImGui::Checkbox("Record game", &mRecordGame);

        if (ImGui::Button("GO", ImVec2(-1.F, 0.F))) {
            static std::string selectedMap = std::to_string(mMapSelector);
            static std::string programName = "fake_program_name";
            char* params[2] = { const_cast<char*>(programName.data()), const_cast<char*>(selectedMap.data()) };
            std::thread t([&params]() { __main(2, params); });
            t.detach();
        }

        ImGui::End();
    }

    {
        ImGui::Begin("Map");

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImVec2 p = ImGui::GetCursorScreenPos();
        draw_list->AddRectFilled(p, ImVec2(p.x + 23 * 34, p.y + 23 * 34), IM_COL32(238, 238, 238, 255));

        for (size_t x = 0; x < 23; ++x) {
            for (size_t y = 0; y < 23; ++y) {
                ImVec2 tl = ImGui::GetCursorScreenPos();
                tl.x += static_cast<float>(x) * 34.f + 1.f;
                tl.y += static_cast<float>(y) * 34.f + 1.f;
                draw_list->AddRectFilled(tl, ImVec2(tl.x + 32, tl.y + 32), IM_COL32(96, 163, 98, 255));
                if (x == 0 || y == 0 || x == 22 || y == 22 || (!(x % 2) && !(y % 2))) {
                    draw_list->AddImage(mBushImage, tl, ImVec2(tl.x + 32, tl.y + 32));
                }
            }
        }

        ImGui::End();
    }

    // // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    // {
    //     static float f = 0.0f;
    //     static int counter = 0;

    //     ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

    //     ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
    //     ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
    //     ImGui::Checkbox("Another Window", &show_another_window);

    //     ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f

    //     if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
    //         counter++;
    //     ImGui::SameLine();
    //     ImGui::Text("counter = %d", counter);

    //     ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0 / static_cast<double>(ImGui::GetIO().Framerate),
    //         static_cast<double>(ImGui::GetIO().Framerate));
    //     ImGui::End();
    // }

    // // 3. Show another simple window.
    // if (show_another_window) {
    //     ImGui::Begin("Another Window",
    //         &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    //     ImGui::Text("Hello from another window!");
    //     if (ImGui::Button("Close Me"))
    //         show_another_window = false;
    //     ImGui::End();
    // }

    // Rendering
    ImGui::Render();

    mRenderLock.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

Framework::Framework()
{
    mBushImage = LoadImage(std::filesystem::path(PROJECT_DIR) / "framework" / "res" / "bush.png");
}

void* Framework::LoadImage(const std::filesystem::path& path)
{
    const char* p = path.c_str();
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(p, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return 0;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    return reinterpret_cast<void*>(static_cast<intptr_t>(image_texture));
}