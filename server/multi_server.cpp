#include "multi_server.h"

MultiServer::MultiServer(const in_port_t port)
    : mPort(port)
{
}

void MultiServer::WaitForConnections(size_t count)
{
    sockpp::tcp_acceptor acc(mPort);
    if (!acc) {
        CHECK(false, "Error creating the acceptor: " + acc.last_error_str());
    }
    std::cout << "Acceptor bound to address: " << acc.address() << std::endl;
    for (size_t conn = 0; conn < count; ++conn) {
        sockpp::inet_address peer;
        sockpp::tcp_socket sock = acc.accept(&peer);
        std::cout << "Received a connection request from " << peer << std::endl;
        if (!sock) {
            CHECK(false, "Error accepting incoming connection: " + acc.last_error_str());
        }
        mClients.push_back(std::move(sock));
    }
}

bool MultiServer::SendToConnection(uint8_t connection, const std::string& message)
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

std::string MultiServer::ReadFromConnection(uint8_t connection)
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

bool MultiServer::IsConnectionValid(uint8_t connection)
{
    return connection < mClients.size() && mClients[connection].is_open();
}

void MultiServer::CloseConnection(uint8_t connection)
{
    if (IsConnectionValid(connection)) {
        mClients[connection].close();
    }
}