#include "parser.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "check.h"

GameDescription parseGameDescription(const std::vector<std::string>& startInfos)
{
    GameDescription description;
    for (const auto& element : startInfos) {
        std::stringstream stream(element);
#ifdef CONSOLE_LOG
        std::cout << "> " << element << std::endl;
#endif

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
            CHECK(false, "Unhandled message: " + msg);
        }
    }
    return description;
}

TickDescription parseTickDescription(const std::vector<std::string>& infos)
{
    TickDescription newDescription;
    for (const auto& element : infos) {
        std::stringstream stream(element);
#ifdef CONSOLE_LOG
        std::cout << "> " << element << std::endl;
#endif

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
            stream >> vampire.mY;
            stream >> vampire.mX;
            stream >> vampire.mHealth;
            stream >> vampire.mPlacableGrenades;
            stream >> vampire.mGrenadeRange;
            stream >> vampire.mRunningShoesTick;
        } else if (msg == "GRENADE") {
            Grenade& grenade = newDescription.mGrenades.emplace_back();
            stream >> grenade.mId;
            stream >> grenade.mY;
            stream >> grenade.mX;
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
                CHECK(false, "Unhandled type: " + type);
            }(t);
            stream >> powerUp.mRemainingTick;
            stream >> powerUp.mY;
            stream >> powerUp.mX;
            stream >> powerUp.mDefensTime;
        } else if (msg == "BAT1") {
            while (true) {
                BatSquad squad;
                squad.mDensity = 1;
                stream >> squad.mY;
                stream >> squad.mX;
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
                stream >> squad.mY;
                stream >> squad.mX;
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
                stream >> squad.mY;
                stream >> squad.mX;
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
            CHECK(false, "Unhandled message: " + msg);
        }
    }
    return newDescription;
}

Answer parseAnswer(const std::vector<std::string>& message)
{
    Answer retVal;

    for (const auto& element : message) {
        std::stringstream stream(element);
        std::string id;
        stream >> id;
        if (id == "RES") {
            continue;
        } else if (id == "GRENADE") {
            retVal.mPlaceGrenade = true;
        } else if (id == "MOVE") {
            while (true) {
                char c = 0;
                stream >> c;
                if (c == 0) {
                    break;
                }
                retVal.mSteps.push_back(c);
            }
        } else if (id == "THROW") {
            std::string dir;
            int length;
            stream >> dir;
            stream >> length;
            Throw::Direction direction = Throw::Direction::Up;
            if (dir == "U") {
                direction = Throw::Direction::Up;
            } else if (dir == "D") {
                direction = Throw::Direction::Down;
            } else if (dir == "L") {
                direction = Throw::Direction::Left;
            } else if (dir == "R") {
                direction = Throw::Direction::Right;
            } else if (dir == "XU") {
                direction = Throw::Direction::XUp;
            } else if (dir == "XD") {
                direction = Throw::Direction::XDown;
            } else if (dir == "XL") {
                direction = Throw::Direction::XLeft;
            } else if (dir == "XR") {
                direction = Throw::Direction::XRight;
            }
            retVal.mThrow = { direction, length };
        }
    }

    return retVal;
}

std::vector<std::string> createAnswer(const Answer& answer, int gameId, int tick, int vampireId)
{
    CHECK(!(answer.mPlaceGrenade && answer.mThrow), "Only one grenade action allowed!");

    std::vector<std::string> commands { "RES " + std::to_string(gameId) + " " + std::to_string(tick) + " " + std::to_string(vampireId) };
    if (answer.mPlaceGrenade) {
        commands.emplace_back("GRENADE");
    }
    if (answer.mThrow) {
        const std::string direction = [](const Throw::Direction& dir) {
            switch (dir) {
            case Throw::Direction::Up:
                return "U";
            case Throw::Direction::Down:
                return "D";
            case Throw::Direction::Left:
                return "L";
            case Throw::Direction::Right:
                return "R";
            case Throw::Direction::XUp:
                return "XU";
            case Throw::Direction::XDown:
                return "XD";
            case Throw::Direction::XLeft:
                return "XL";
            case Throw::Direction::XRight:
                return "XR";
            }
            CHECK(false, "Unhandled type");
            return "";
        }(answer.mThrow->mDirection);
        commands.emplace_back("THROW " + direction + " " + std::to_string(answer.mThrow->mDistance));
    }
    if (!answer.mSteps.empty()) {
        std::string moveCommand = "MOVE";
        for (const auto& step : answer.mSteps) {
            moveCommand += std::string(" ") + step;
        }
        commands.emplace_back(moveCommand);
    }

    return commands;
}
