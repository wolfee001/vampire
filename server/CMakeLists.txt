add_subdirectory(sockpp)

add_executable(server server.cpp ../simulator.cpp ../parser.cpp)
target_link_libraries(server project_options sockpp-static CONAN_PKG::fmt)