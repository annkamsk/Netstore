
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

class IConnection {
public:
    virtual void openSocket() = 0;
    virtual std::vector<char> readFromSocket() = 0;

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

    std::vector<char> readFromSocket() override;
};

class TCPConnection : public Connection {
public:
    TCPConnection() = default;
    void openSocket() override;
    std::vector<char> readFromSocket() override;
};
#endif //SIK2_CONNECTION_H
