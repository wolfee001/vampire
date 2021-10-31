#include <_types/_uint8_t.h>
#include <fmt/format.h>
#include <iostream>
#include <sockpp/tcp_acceptor.h>
#include <sockpp/version.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/_types/_in_port_t.h>

#include "../parser.h"
#include "../simulator.h"

class MultiServer {
public:
    MultiServer(const in_port_t port)
        : mPort(port)
    {
    }

    void WaitForConnections(size_t count)
    {
        sockpp::tcp_acceptor acc(mPort);
        if (!acc) {
            throw std::runtime_error("Error creating the acceptor: " + acc.last_error_str());
        }
        std::cout << "Acceptor bound to address: " << acc.address() << std::endl;
        for (size_t conn = 0; conn < count; ++conn) {
            sockpp::inet_address peer;
            sockpp::tcp_socket sock = acc.accept(&peer);
            std::cout << "Received a connection request from " << peer << std::endl;
            if (!sock) {
                throw std::runtime_error("Error accepting incoming connection: " + acc.last_error_str());
            }
            mClients.push_back(std::move(sock));
        }
    }

    bool SendToConnection(uint8_t connection, const std::string& message)
    {
        sockpp::tcp_socket& sock = mClients[connection];
        if (!sock.is_open()) {
            return false;
        }
        if (sock.write(message) <= 0) {
            sock.close();
            return false;
        }
        return true;
    }

    std::string ReadFromConnection(uint8_t connection)
    {
        sockpp::tcp_socket& sock = mClients[connection];
        if (!sock.is_open()) {
            return "";
        }
        static char buffer[65535];
        memset(buffer, 0, 65535);
        int n = -1;
        if ((n = sock.read(buffer, 65535)) <= 0) {
            sock.close();
            return "";
        }
        return std::string(buffer, n);
    }

    bool IsConnectionValid(uint8_t connection)
    {
        return mClients[connection].is_open();
    }

private:
    sockpp::socket_initializer mSockInit;
    in_port_t mPort;
    std::vector<sockpp::tcp_socket> mClients;
};

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
            //     throw std::runtime_error("huha...");
            // }
            return result;
        }
        if (!line.empty()) {
            result.push_back(line);
        }
    }
    return {};
}

Answer CreateAnswer(const std::vector<std::string>& message)
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
        }
    }

    return retVal;
}

std::vector<std::string> CreateInfo(const TickDescription& tick, int player)
{
    std::vector<std::string> retVal;
    retVal.push_back(fmt::format("REQ {} {} {}", tick.mRequest.mGameId, tick.mRequest.mTick, player));
    retVal.push_back(fmt::format("VAMPIRE 1 {} {} {} {} {} {}", tick.mMe.mX, tick.mMe.mY, tick.mMe.mHealth, tick.mMe.mPlacableGrenades, tick.mMe.mGrenadeRange,
        tick.mMe.mRunningShoesTick));
    for (const auto& vampire : tick.mEnemyVampires) {
        retVal.push_back(fmt::format("VAMPIRE {} {} {} {} {} {} {}", vampire.mId + 2, vampire.mX, vampire.mY, vampire.mHealth, vampire.mPlacableGrenades,
            vampire.mGrenadeRange, vampire.mRunningShoesTick));
    }
    for (const auto& grenade : tick.mGrenades) {
        retVal.push_back(fmt::format("GRENADE {} {} {} {} {}", grenade.mId, grenade.mX, grenade.mY, grenade.mTick, grenade.mRange));
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
        }(powerup.mType);
        retVal.push_back(fmt::format("POWERUP {} {} {} {}", type, powerup.mRemainingTick, powerup.mX, powerup.mY));
    }
    std::string bat1 = "BAT1";
    for (const auto& bat : tick.mBat1) {
        bat1 += fmt::format(" {} {}", bat.mX, bat.mY);
    }
    retVal.push_back(bat1);
    std::string bat2 = "BAT2";
    for (const auto& bat : tick.mBat2) {
        bat2 += fmt::format(" {} {}", bat.mX, bat.mY);
    }
    retVal.push_back(bat2);
    std::string bat3 = "BAT3";
    for (const auto& bat : tick.mBat3) {
        bat3 += fmt::format(" {} {}", bat.mX, bat.mY);
    }
    retVal.push_back(bat3);

    return retVal;
}

int main()
{
    std::vector<std::string> startInfo = {
        "MESSAGE OK",
        "LEVEL 1",
        "GAMEID 55544",
        "TEST 1",
        "MAXTICK 500",
        "GRENADERADIUS 2",
        "SIZE 11",
    };
    std::vector<std::string> info
        = { "REQ 775 0 1", "VAMPIRE 1 1 1 3 1 2 0", "BAT1 4 1 5 1 6 1 3 2 7 2 2 3 3 3 7 3 8 3 1 4 9 4 1 5 9 5 1 6 9 6 2 7 3 7 7 7 8 7 3 8 7 8 4 9 5 9 6 9",
              "BAT2 5 2 4 3 6 3 3 4 7 4 2 5 8 5 3 6 7 6 4 7 6 7 5 8", "BAT3 5 3 5 4 3 5 4 5 5 5 6 5 7 5 5 6 5 7" };

    GameDescription gd = parseGameDescription(startInfo);
    TickDescription tick = parseTickDescription(info);

    Simulator simulator(gd);
    simulator.SetState(tick);

    const int playerCount = 1;
    MultiServer ms(6789);
    ms.WaitForConnections(playerCount);
    for (int i = 0; i < playerCount; ++i) {
        ms.ReadFromConnection(i);
        ms.SendToConnection(i, CreateMessage(startInfo));
    }

    for (int i = 0; i < gd.mMaxTick; ++i) {
        for (int p = 0; p < playerCount; ++p) {
            ms.SendToConnection(p, CreateMessage(CreateInfo(tick, p + 1)));
        }
        for (int p = 0; p < playerCount; ++p) {
            Answer ans = CreateAnswer(ParseMessage(ms.ReadFromConnection(p)));
            simulator.SetVampireMove(p + 1, ans);
        }
        tick = simulator.Tick();
        simulator.SetState(tick);
    }

    for (int i = 0; i < playerCount; ++i) {
        ms.SendToConnection(i, "END 666 Ran_out_of_ticks\n.\n");
    }

    return 0;
}
