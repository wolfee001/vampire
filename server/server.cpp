#include "../parser.h"

#include "game.h"
#include "levels.h"
#include "multi_server.h"
#include "parser.h"

#include <algorithm>
#include <fmt/format.h>

void Run(int playerCount, const Level& level)
{
    GameDescription gd = parseGameDescription(level.mGameDescription);
    TickDescription tick = parseTickDescription(level.mZeroTick);

    tick.mEnemyVampires.erase(
        std::remove_if(tick.mEnemyVampires.begin(), tick.mEnemyVampires.end(), [&playerCount](const auto& vampire) { return vampire.mId > playerCount; }),
        tick.mEnemyVampires.end());

    MultiServer ms(6789);
    ms.WaitForConnections(playerCount);
    for (int i = 0; i < playerCount; ++i) {
        ms.ReadFromConnection(i);
        ms.SendToConnection(i, CreateMessage(level.mGameDescription));
    }

    Game game(level, playerCount);

    while (true) {
        for (int p = 0; p < playerCount; ++p) {
            ms.SendToConnection(p, CreateMessage(CreateInfo(tick, p + 1)));
        }
        for (int p = 0; p < playerCount; ++p) {
            Answer ans = parseAnswer(ParseMessage(ms.ReadFromConnection(p)));
            game.SetVampireMove(p + 1, ans);
        }
        std::pair<TickDescription, std::vector<std::pair<int, float>>> gameState = game.Tick();

        for (const auto& deadVampire : gameState.second) {
            if (ms.IsConnectionValid(deadVampire.first - 1)) {
                ms.SendToConnection(deadVampire.first - 1, fmt::format("END {} Could_not_be_more_deaded\n.\n", deadVampire.second));
                ms.CloseConnection(deadVampire.first - 1);
            }
        }

        const auto aliveVampire = std::find_if(
            gameState.first.mEnemyVampires.begin(), gameState.first.mEnemyVampires.end(), [](const auto& element) { return element.mHealth > 0; });
        if (gameState.first.mMe.mHealth < 1 && aliveVampire == gameState.first.mEnemyVampires.end()) {
            break;
        }

        tick = gameState.first;
    }
}
