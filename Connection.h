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

    const static unsigned BUFFER_LEN = 2048;
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

class Connection {
protected:
    static const unsigned N = 50;
    std::string mcast{};
    std::string local;
    uint16_t port{};
    unsigned int ttl{};

    struct ip_mreq ip_mreq{}; // for server for listening to group
    struct sockaddr_in remote_address{}; // for client for sending to group

public:
    Connection() = default;

    Connection(std::string mcast, uint16_t port, unsigned int ttl) : mcast(std::move(mcast)),
                                                                     port(port),
                                                                     ttl(ttl) {}

    unsigned getTTL() {
        return this->ttl;
    }

    std::string getMcast() const {
        return this->mcast;
    }

    uint16_t getPort() const {
        return port;
    }

    int openUDPSocket();

    int openTCPSocket();

    static ConnectionResponse readFromUDPSocket(int sock);

    static void sendToSocket(int sock, sockaddr_in address, std::string data);

    static int getPort(int sock) {
        struct sockaddr_in sin{};
        int addrlen = sizeof(sin);
        getsockname(sock, (struct sockaddr *)&sin, (socklen_t *) &addrlen);
        return sin.sin_port;
    }

    static void closeSocket(int sock);

    static std::vector<char> receiveFile(int sock);

    void setReceiver();

    void multicast(int sock, std::string data);

    void addToMulticast(int sock);
};

#endif //SIK2_CONNECTION_H
