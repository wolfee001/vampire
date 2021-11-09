add_subdirectory(sockpp)

add_executable(server
    game.cpp
    gui.cpp
    levels.cpp
    main.cpp
    multi_server.cpp
    parser.cpp
    server.cpp
)
target_link_libraries(server
    project_options
    sockpp-static
    libhelper
    CONAN_PKG::fmt
    libimguibindings
    CONAN_PKG::imgui
    CONAN_PKG::stb
    CONAN_PKG::fmt
)

target_compile_definitions(server PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(server PRIVATE GL_SILENCE_DEPRECATION)