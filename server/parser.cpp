#include "parser.h"

#include <fmt/format.h>
#include <sstream>

#include "../check.h"

std::string CreateMessage(const std::vector<std::string>& message)
{
    std::string retVal;
    for (const auto& element : message) {
        std::cout << "< " << element << std::endl;
        retVal += element + "\n";
    }
    retVal += ".\n";
    return retVal;
}

std::vector<std::string> ParseMessage(const std::string& message)
{
    std::cout << "> " << message << std::endl;
    std::vector<std::string> result;
    std::string line;
    std::stringstream consumer(message);
    while (std::getline(consumer, line)) {
        if (line == ".") {
            // if (!consumer.eof()) {
            //     CHECK(false, "huha...");
            // }
            return result;
        }
        if (!line.empty()) {
            result.push_back(line);
        }
    }
    return {};
}

std::vector<std::string> CreateGameDescription(const GameDescription& description)
{
    std::vector<std::string> retVal;
    retVal.push_back("MESSAGE OK");
    retVal.push_back(fmt::format("LEVEL {}", description.mLevelId));
    retVal.push_back(fmt::format("GAMEID {}", description.mGameId));
    retVal.push_back("TEST 1");
    retVal.push_back(fmt::format("MAXTICK {}", description.mMaxTick));
    retVal.push_back(fmt::format("GRENADERADIUS {}", description.mGrenadeRadius));
    retVal.push_back(fmt::format("SIZE {}", description.mMapSize));
    return retVal;
}

std::vector<std::string> CreateInfo(const TickDescription& tick, int player)
{
    std::vector<std::string> retVal;
    retVal.push_back(fmt::format("REQ {} {} {}", tick.mRequest.mGameId, tick.mRequest.mTick, player));
    retVal.push_back(fmt::format("VAMPIRE 1 {} {} {} {} {} {}", tick.mMe.mY, tick.mMe.mX, tick.mMe.mHealth, tick.mMe.mPlacableGrenades, tick.mMe.mGrenadeRange,
        tick.mMe.mRunningShoesTick));
    for (const auto& vampire : tick.mEnemyVampires) {
        retVal.push_back(fmt::format("VAMPIRE {} {} {} {} {} {} {}", vampire.mId, vampire.mY, vampire.mX, vampire.mHealth, vampire.mPlacableGrenades,
            vampire.mGrenadeRange, vampire.mRunningShoesTick));
    }
    for (const auto& grenade : tick.mGrenades) {
        retVal.push_back(fmt::format("GRENADE {} {} {} {} {}", grenade.mId, grenade.mY, grenade.mX, grenade.mTick, grenade.mRange));
    }
    for (const auto& powerup : tick.mPowerUps) {
        std::string type = [](PowerUp::Type type) {
            switch (type) {
            case PowerUp::Type::Battery:
                return "BATTERY";
            case PowerUp::Type::Grenade:
                return "GRENADE";
            case PowerUp::Type::Shoe:
                return "SHOE";
            case PowerUp::Type::Tomato:
                return "TOMATO";
            }
            CHECK(false, "unhandled type!");
        }(powerup.mType);
        retVal.push_back(fmt::format("POWERUP {} {} {} {} {}", type, powerup.mRemainingTick, powerup.mY, powerup.mX, powerup.mDefensTime));
    }
    if (!tick.mBat1.empty()) {
        std::string bat1 = "BAT1";
        for (const auto& bat : tick.mBat1) {
            bat1 += fmt::format(" {} {}", bat.mY, bat.mX);
        }
        retVal.push_back(bat1);
    }
    if (!tick.mBat2.empty()) {
        std::string bat2 = "BAT2";
        for (const auto& bat : tick.mBat2) {
            bat2 += fmt::format(" {} {}", bat.mY, bat.mX);
        }
        retVal.push_back(bat2);
    }
    if (!tick.mBat3.empty()) {
        std::string bat3 = "BAT3";
        for (const auto& bat : tick.mBat3) {
            bat3 += fmt::format(" {} {}", bat.mY, bat.mX);
        }
        retVal.push_back(bat3);
    }

    return retVal;
}