#pragma once

#include <sockpp/tcp_acceptor.h>

#include "../check.h"

class MultiServer {
public:
    MultiServer(const in_port_t port);

    void WaitForConnections(size_t count);
    bool SendToConnection(uint8_t connection, const std::string& message);
    std::string ReadFromConnection(uint8_t connection);
    bool IsConnectionValid(uint8_t connection);
    void CloseConnection(uint8_t connection);

private:
    sockpp::socket_initializer mSockInit;
    in_port_t mPort;
    std::vector<sockpp::tcp_socket> mClients;
};
