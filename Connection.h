#ifndef SIK2_CONNECTION_H
#define SIK2_CONNECTION_H

#include <string>
#include <vector>
#include <list>
#include <utility>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <functional>
#include <iostream>
#include <cstring>
#include <poll.h>

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

    virtual int getSock() = 0;

    virtual void sendToSocket(struct sockaddr_in rec, std::string data) = 0;

    virtual std::string getMcast() const = 0;

};

class Connection : public IConnection {
protected:
    static const unsigned N = 50;
    std::string mcast{};
    std::string local;
    uint16_t port{};
    unsigned int ttl{};
    struct ip_mreq ip_mreq{};
    int masterSock{}, sockets[N]{};

public:
    Connection() = default;

    Connection(std::string mcast, uint16_t port, unsigned int ttl) : mcast(std::move(mcast)),
                                                                     port(port),
                                                                     ttl(ttl) {}

    int getSock() override {
        return this->masterSock;
    }

    unsigned getTTL() {
        return this->ttl;
    }

    std::string getMcast() const override {
        return this->mcast;
    }

    uint16_t getPort() const {
        return port;
    }
    virtual void addToLocal(unsigned port);

    void detachFromGroup();

    void closeSocket();

    void addToMcast();

    void activateBroadcast();

    void setReceiver();

    virtual void broadcast(std::string data);

    ConnectionResponse waitForResponse();
};

class UDPConnection : public Connection {
public:
    UDPConnection(std::string mcast, uint16_t port, unsigned int ttl) : Connection(std::move(mcast), port, ttl) {}

    void openSocket() override;

    ConnectionResponse readFromSocket() override;

    void sendToSocket(struct sockaddr_in rec, std::string data) override;

    void broadcast(std::string data) override;
};

class TCPConnection : public Connection {
    static const int N = 20;
    struct pollfd fds[N]{ -1, POLLIN, 0 };
public:
    TCPConnection() = default;

    void openSocket() override;
    void addToLocal(unsigned port) override;

    void setToListen();

    ConnectionResponse readFromSocket() override;

};

#endif //SIK2_CONNECTION_H
