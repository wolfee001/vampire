#include <cstring>
#include <iostream>

#include "gui.h"
#include "server.h"

void PrintHelp(const char* program)
{
    std::cerr << "Usage: " << std::endl
              << program << " help                         "
              << "\tPrint this message" << std::endl
              << program << " gui                          "
              << "\tRuns in gui mode" << std::endl
              << program << " [level] [playercount] [seed] "
              << "\tPlay with [level] level (1-10), [playercount] number of players with the seed of [seed]. Seed is optional." << std::endl;
}

int main(int argc, char** argv)
{
    if (argc > 1 && 0 == std::strcmp("help", argv[1])) {
        PrintHelp(argv[0]);
        return 0;
    } else if (argc == 1 || (argc > 1 && 0 == std::strcmp("gui", argv[1]))) {
        GUI::GetInstance().Run();
        return 0;
    } else if (argc == 3) {
        Levels levels;
        RunGame(std::atoi(argv[2]), levels.mLevels[std::atoi(argv[1]) - 1], time(nullptr));
        return 0;
    } else if (argc == 4) {
        Levels levels;
        RunGame(std::atoi(argv[2]), levels.mLevels[std::atoi(argv[1]) - 1], std::atoi(argv[3]));
        return 0;
    } else {
        PrintHelp(argv[0]);
        return -1;
    }
}