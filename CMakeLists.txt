cmake_minimum_required(VERSION 3.16)

project(vampire)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_compile_options(-fstandalone-debug)
endif()

add_compile_definitions(CHECK_ENABLED)

if(TICK_TIMEOUT)
    add_compile_definitions(TIMEOUT=${TICK_TIMEOUT})
endif()

#-------------- SETUPS
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

include(cmake/CompilerWarnings.cmake)
add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

include(cmake/StaticAnalyzers.cmake)
setup_static_analyzers(CLANG_TIDY)

include(cmake/Conan.cmake)
setup_conan()

enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(MAKE_DIRECTORY target)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/data)
#--------------

#-------------- NORMAL BUILD
add_library(libhelper
    parser.cpp
    simulator.cpp
    search.cpp
)
target_link_libraries(libhelper
    PUBLIC project_options
    PRIVATE project_warnings
    PRIVATE CONAN_PKG::boost
)

add_library(libmagic
    i_magic.cpp
    usual_magic.cpp
    gabor_magic.cpp
    final_magic.cpp
)
target_link_libraries(libmagic
    PUBLIC project_options
    PRIVATE CONAN_PKG::boost
    PRIVATE libhelper
)

add_library(libsolver
    solver.cpp
)
target_link_libraries(libsolver
    PUBLIC project_options
    PRIVATE project_warnings
    PRIVATE libmagic
    PRIVATE libhelper
)

add_executable(vampire main.cpp)
target_link_libraries(vampire
    PRIVATE project_options
    PRIVATE libsolver
)
#--------------

#-------------- TEST BUILD
enable_testing()

add_executable(unittest
    test/main.cpp
    test/test_solver.cpp
    test/test_simulator.cpp
    test/test_action_sequence.cpp
    test/test_usual_magic.cpp
    test/test_search.cpp
    test/test_final.cpp
)
target_link_libraries(unittest
    PUBLIC project_options
    PRIVATE project_warnings
    PRIVATE libsolver
    PRIVATE CONAN_PKG::GTest
)

include(GoogleTest)
gtest_discover_tests(unittest)

add_test(NAME unittest COMMAND unittest)
#--------------

#-------------- FRAMEWORK BUILD
add_library(libimguibindings
    ${CMAKE_BINARY_DIR}/bindings/imgui_impl_opengl2.cpp
    ${CMAKE_BINARY_DIR}/bindings/imgui_impl_glfw.cpp
)
target_include_directories(libimguibindings INTERFACE ${CMAKE_BINARY_DIR}/bindings/)
target_link_libraries(libimguibindings
    PUBLIC project_options
    PUBLIC CONAN_PKG::imgui
    PUBLIC CONAN_PKG::glfw
)

add_executable(framework
    framework/framework.cpp
    framework/game_loader.cpp
    framework/game_recorder.cpp
    framework/gui_element.cpp
    framework/main.cpp
    main.cpp
    solver.cpp
)
target_link_libraries(framework
    PRIVATE project_options
    PRIVATE libimguibindings
    PRIVATE CONAN_PKG::imgui
    PRIVATE CONAN_PKG::stb
    PRIVATE CONAN_PKG::fmt
    PRIVATE CONAN_PKG::portable-file-dialogs
    PRIVATE libmagic
    PRIVATE libhelper
)
target_compile_definitions(framework PRIVATE GAME_WITH_FRAMEWORK)
target_include_directories(framework INTERFACE framework/)
target_compile_definitions(framework PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(framework PRIVATE GL_SILENCE_DEPRECATION)
#--------------

add_subdirectory(server)