#ifndef SIK2_CONNECTION_H
#define SIK2_CONNECTION_H

#include "err.h"
#include "Message.h"

class ConnectionResponse {
    std::vector<my_byte> buffer{};
    struct sockaddr_in cliaddr{};
    size_t size{};
public:

    ConnectionResponse() = default;
    const std::vector<my_byte> &getBuffer() const {
        return buffer;
    }

    size_t getSize() const {
        return size;
    }

    void setSize(size_t size) {
        ConnectionResponse::size = size;
    }

    const sockaddr_in &getCliaddr() const {
        return cliaddr;
    }

    void setBuffer(const std::vector<my_byte> &buffer) {
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
    struct sockaddr_in remote_address{}; // for client for senging to group

public:
    Connection() = default;

    Connection(std::string mcast, uint16_t port, unsigned int ttl) : mcast(std::move(mcast)),
                                                                     port(port),
                                                                     ttl(ttl) {}
    int openUDPSocket();

    int openTCPSocket();

    int openTCPSocket(struct sockaddr_in provider);

    static ConnectionResponse readFromUDPSocket(int sock);

    static void sendToSocket(int sock, sockaddr_in address, const std::shared_ptr<Message>& message);

    static int getPort(int sock) {
        struct sockaddr_in sin{};
        int addrlen = sizeof(sin);
        getsockname(sock, (struct sockaddr *)&sin, (socklen_t *) &addrlen);
        return sin.sin_port;
    }

    static void closeSocket(int sock);

    static void receiveFile(int sock, FILE *file);

    void sendFile(int sock, const std::string& path);

    void multicast(int sock, const std::shared_ptr<Message>& message);

    void addToMulticast(int sock);

    void setReceiver();

    std::string getMcast() const {
        return this->mcast;
    }

    unsigned int getTtl() const {
        return this->ttl;
    }
};

#endif //SIK2_CONNECTION_H
