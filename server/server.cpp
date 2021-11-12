#include "../parser.h"

#include "game.h"
#include "gui.h"
#include "levels.h"
#include "multi_server.h"
#include "parser.h"

#include <algorithm>
#include <ctime>
#include <fmt/format.h>
#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>

void RunGame(int playerCount, const Level& level, int seed)
{
    srand(seed);
    GameDescription gd = parseGameDescription(level.mGameDescription);
    gd.mGameId = std::time(nullptr);
    TickDescription tick = parseTickDescription(level.mZeroTick);
    tick.mRequest.mGameId = gd.mGameId;

    tick.mEnemyVampires.erase(
        std::remove_if(tick.mEnemyVampires.begin(), tick.mEnemyVampires.end(), [&playerCount](const auto& vampire) { return vampire.mId > playerCount; }),
        tick.mEnemyVampires.end());

    struct Player {
        std::string name;
        int lastTick = -1;
        float score = 0.F;
    };

    std::map<int, Player> players;

    MultiServer ms(6789);
    ms.WaitForConnections(playerCount);
    for (int i = 0; i < playerCount; ++i) {
        std::stringstream ss(ms.ReadFromConnection(i));
        std::string s;
        ss >> s >> s;
        players[i + 1].name = s;
        GUI::GetInstance().SetVampireName(i + 1, s);
        ms.SendToConnection(i, CreateMessage(CreateGameDescription(gd)));
    }

    Game game(gd, tick, playerCount);

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
                players[deadVampire.first].lastTick = gameState.first.mRequest.mTick;
                players[deadVampire.first].score = deadVampire.second;
                if (gameState.first.mRequest.mTick > gd.mMaxTick) {
                    players[deadVampire.first].score += 144.F;
                } else {
                    players[deadVampire.first].score += static_cast<float>(gameState.first.mRequest.mTick) / static_cast<float>(gd.mMaxTick) * 144.F;
                }
            }
        }

        std::vector<Vampire*> survivors;
        if (gameState.first.mMe.mHealth > 0) {
            survivors.push_back(&gameState.first.mMe);
        }
        for (auto& v : gameState.first.mEnemyVampires) {
            if (v.mHealth > 0) {
                survivors.push_back(&v);
            }
        }

        if (survivors.empty()) {
            break;
        }
        if (survivors.size() == 1 && playerCount != 1) {
            players[survivors[0]->mId].score = game.GetPoint(survivors[0]->mId) + 144.F;
            ms.SendToConnection(survivors[0]->mId - 1, fmt::format("END {} Survivor\n.\n", players[survivors[0]->mId].score));
            ms.CloseConnection(survivors[0]->mId - 1);
            break;
        }

        tick = gameState.first;
    }

    nlohmann::json gameJson;
    gameJson["id"] = gd.mGameId;
    gameJson["level"] = gd.mLevelId;
    gameJson["maxTick"] = gd.mMaxTick;
    gameJson["size"] = gd.mMapSize;
    gameJson["seed"] = seed;

    nlohmann::json results;
    for (const auto& [id, player] : players) {
        nlohmann::json r;
        r["player"] = id;
        r["version"] = player.name;
        r["lastTick"] = player.lastTick;
        r["score"] = player.score;
        results.push_back(r);
    }

    nlohmann::json root;
    root["game"] = gameJson;
    root["results"] = results;

    std::ofstream json("result.json");
    json << std::setw(2) << root;
    json.close();
}
