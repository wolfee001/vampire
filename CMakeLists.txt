cmake_minimum_required(VERSION 3.20)

project(vampire)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

#--------------
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

add_library(libmagic usual_magic.cpp)
target_link_libraries(libmagic
    PUBLIC project_options
    PRIVATE CONAN_PKG::boost
)

add_library(libvampire solver.cpp)
target_link_libraries(libvampire
    PUBLIC project_options
    PRIVATE project_warnings
    PRIVATE libmagic
)

add_executable(vampire main.cpp)
target_link_libraries(vampire
    PRIVATE project_options
    PRIVATE libvampire
)
