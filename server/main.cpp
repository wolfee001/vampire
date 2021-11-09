#include <cstring>
#include <iostream>

#include "gui.h"
#include "server.h"

void PrintHelp(const char* program)
{
    std::cerr << "Usage: " << std::endl
              << program << " help                   "
              << "\tPrint this message" << std::endl
              << program << " gui                    "
              << "\tRuns in gui mode" << std::endl
              << program << " [level] [playercount]  "
              << "\tPlay with [level] level (1-10), [playercount] number of players " << std::endl;
}

int main(int argc, char** argv)
{
    srand(time(nullptr));
    if (argc > 1 && 0 == std::strcmp("help", argv[1])) {
        PrintHelp(argv[0]);
        return 0;
    } else if (argc == 1 || (argc > 1 && 0 == std::strcmp("gui", argv[1]))) {
        GUI::GetInstance().Run();
        return 0;
    } else if (argc == 3) {
        Levels levels;
        RunGame(std::atoi(argv[2]), levels.mLevels[std::atoi(argv[1]) - 1]);
        return 0;
    } else {
        PrintHelp(argv[0]);
        return -1;
    }
}