#include <utility>


#ifndef SIK2_CONNECTION_H
#define SIK2_CONNECTION_H
#include <string>
#include <vector>
#include <list>
#include <utility>
#include <map>
#include <algorithm>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <functional>
#include <iostream>

#include "err.h"

namespace Netstore {
    const static int MIN_SMPL_CMD_SIZE = 18;
    const static int MIN_CMPLX_CMD_SIZE = 26;
    /* max SIMPL_CMD size = 10 (cmd) + 8 (seq) + 256 (max filename length) */
    const static unsigned MAX_SMPL_CMD_SIZE = 274;
}

class ConnectionResponse {
    std::vector<char> buffer{};
    struct sockaddr_in cliaddr{};
public:
    ConnectionResponse() = default;

    const std::vector<char> &getBuffer() const {
        return buffer;
    }

    const char &getBufferAddr() const {
        return buffer[0];
    }

    const sockaddr_in &getCliaddr() const {
        return cliaddr;
    }

    void setBuffer(const std::vector<char> &buffer) {
        ConnectionResponse::buffer = buffer;
    }

    void setCliaddr(const sockaddr_in &cliaddr) {
        ConnectionResponse::cliaddr = cliaddr;
    }

};

class IConnection {
public:
    virtual void openSocket() = 0;
    virtual ConnectionResponse readFromSocket() = 0;

};
class Connection : public IConnection {
protected:
    std::string ip;
    int sock{};
    int port{};
    struct ip_mreq ip_mreq{};


public:
    Connection() = default;
    Connection(uint16_t port) : port(port){};
    void addToLocal();
    void detachFromGroup();
    void closeSocket();

    void addToMcast(std::string mcast);
};

class UDPConnection : public Connection {
public:
    UDPConnection(uint16_t port) : Connection(port){};
    void openSocket() override;

    ConnectionResponse readFromSocket() override;
};

class TCPConnection : public Connection {
public:
    TCPConnection() = default;
    void openSocket() override;
    ConnectionResponse readFromSocket() override;
};
#endif //SIK2_CONNECTION_H
