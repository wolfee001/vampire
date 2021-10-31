cmake_minimum_required(VERSION 3.16)

project(vampire)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
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
#--------------

#-------------- NORMAL BUILD
add_library(libmagic usual_magic.cpp)
target_link_libraries(libmagic
    PUBLIC project_options
    PRIVATE CONAN_PKG::boost
)

add_library(libsolver
    solver.cpp
    simulator.cpp
    mcts.cpp
)
target_link_libraries(libsolver
    PUBLIC project_options
    PRIVATE project_warnings
    PRIVATE libmagic
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
)
target_link_libraries(unittest
    PUBLIC project_options
    PRIVATE project_warnings
    PRIVATE libsolver
    PRIVATE CONAN_PKG::gtest
)

include(GoogleTest)
gtest_discover_tests(unittest)

add_test(NAME unittest COMMAND unittest)
#--------------

#-------------- FRAMEWORK BUILD
add_library(libframework
    framework/game_recorder.cpp
    framework/framework.cpp
    framework/gui_element.cpp
)
target_link_libraries(libframework
    PUBLIC project_options
    PRIVATE project_warnings
    PUBLIC CONAN_PKG::imgui
    PRIVATE CONAN_PKG::stb
    PRIVATE CONAN_PKG::fmt
)
target_include_directories(libframework INTERFACE framework/)
target_compile_definitions(libframework PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(libframework PRIVATE GL_SILENCE_DEPRECATION)

add_library(libsolver_fw solver.cpp)
target_link_libraries(libsolver_fw
    PUBLIC project_options
    PRIVATE project_warnings
    PRIVATE libmagic
    PRIVATE libframework
)
target_compile_definitions(libsolver_fw PRIVATE GAME_WITH_FRAMEWORK)

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

add_executable(framework framework/main.cpp main.cpp)
target_link_libraries(framework
    PRIVATE project_options
    PRIVATE libimguibindings
    PRIVATE libsolver_fw
)
target_compile_definitions(framework PRIVATE GAME_WITH_FRAMEWORK)
#--------------