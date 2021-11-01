#include "parser.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

GameDescription parseGameDescription(const std::vector<std::string>& startInfos)
{
    GameDescription description;
    for (const auto& element : startInfos) {
        std::stringstream stream(element);

        std::string msg;
        stream >> msg;
        if (msg == "MESSAGE") {
            std::string message;
            while (true) {
                std::string tmp;
                stream >> tmp;
                if (!tmp.empty()) {
                    message += " " + tmp;
                } else {
                    break;
                }
            }
            if (message != " OK") {
                std::cerr << "Error with authentication: " << message << std::endl;
                throw std::runtime_error("Error with authentication: " + message);
            }
        } else if (msg == "LEVEL") {
            stream >> description.mLevelId;
        } else if (msg == "GAMEID") {
            stream >> description.mGameId;
        } else if (msg == "TEST") {
            stream >> description.mIsTest;
        } else if (msg == "MAXTICK") {
            stream >> description.mMaxTick;
        } else if (msg == "GRENADERADIUS") {
            stream >> description.mGrenadeRadius;
        } else if (msg == "SIZE") {
            stream >> description.mMapSize;
        } else {
            std::cerr << "Unhandled message: " << msg << std::endl;
            throw std::runtime_error("Unhandled message: " + msg);
        }
    }
    return description;
}

TickDescription parseTickDescription(const std::vector<std::string>& infos)
{
    TickDescription newDescription;
    for (const auto& element : infos) {
        std::stringstream stream(element);

        std::string msg;
        stream >> msg;

        if (msg == "REQ") {
            stream >> newDescription.mRequest.mGameId;
            stream >> newDescription.mRequest.mTick;
            stream >> newDescription.mRequest.mVampireId;
        } else if (msg == "WARN") {
            std::string m;
            stream >> m;
            newDescription.mWarnings.emplace_back(m);
        } else if (msg == "VAMPIRE") {
            int id;
            stream >> id;

            Vampire& vampire = id == newDescription.mRequest.mVampireId ? newDescription.mMe : newDescription.mEnemyVampires.emplace_back();
            vampire.mId = id;
            stream >> vampire.mX;
            stream >> vampire.mY;
            stream >> vampire.mHealth;
            stream >> vampire.mPlacableGrenades;
            stream >> vampire.mGrenadeRange;
            stream >> vampire.mRunningShoesTick;
        } else if (msg == "GRENADE") {
            Grenade& grenade = newDescription.mGrenades.emplace_back();
            stream >> grenade.mId;
            stream >> grenade.mX;
            stream >> grenade.mY;
            stream >> grenade.mTick;
            stream >> grenade.mRange;
        } else if (msg == "POWERUP") {
            std::string t;
            stream >> t;

            PowerUp& powerUp = newDescription.mPowerUps.emplace_back();
            powerUp.mType = [](const std::string& type) -> PowerUp::Type {
                if (type == "TOMATO") {
                    return PowerUp::Type::Tomato;
                }
                if (type == "GRENADE") {
                    return PowerUp::Type::Grenade;
                }
                if (type == "BATTERY") {
                    return PowerUp::Type::Battery;
                }
                if (type == "SHOE") {
                    return PowerUp::Type::Shoe;
                }
                std::cerr << "Unhandled type: " << type << std::endl;
                throw std::runtime_error("Unhandled type: " + type);
            }(t);
            stream >> powerUp.mRemainingTick;
            stream >> powerUp.mX;
            stream >> powerUp.mY;
        } else if (msg == "BAT1") {
            while (true) {
                BatSquad squad;
                squad.mDensity = 1;
                stream >> squad.mX;
                stream >> squad.mY;
                if (squad.mX == -1) {
                    break;
                }
                newDescription.mBat1.push_back(squad);
                newDescription.mAllBats.push_back(squad);
            }
        } else if (msg == "BAT2") {
            while (true) {
                BatSquad squad;
                squad.mDensity = 2;
                stream >> squad.mX;
                stream >> squad.mY;
                if (squad.mX == -1) {
                    break;
                }
                newDescription.mBat2.push_back(squad);
                newDescription.mAllBats.push_back(squad);
            }
        } else if (msg == "BAT3") {
            while (true) {
                BatSquad squad;
                squad.mDensity = 3;
                stream >> squad.mX;
                stream >> squad.mY;
                if (squad.mX == -1) {
                    break;
                }
                newDescription.mBat3.push_back(squad);
                newDescription.mAllBats.push_back(squad);
            }
        } else if (msg == "END") {
            stream >> newDescription.mEndMessage.mPoint;
            stream >> newDescription.mEndMessage.mReason;
        } else {
            std::cerr << "Unhandled message: " << msg << std::endl;
            throw std::runtime_error("Unhandled message: " + msg);
        }
    }
    return newDescription;
}