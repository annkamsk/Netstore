
#include "Connection.h"

int Connection::openUDPSocket() {
    /* open the socket */
    int sock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        syserr("socket");
    }

    /* bind */
    struct sockaddr_in local_address{};
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(this->port);
    if (bind(sock, (struct sockaddr *) &local_address, sizeof local_address) < 0) {
        syserr("bind");
    }
    return sock;
}

void Connection::addToMulticast(int sock) {
    /* allow port to be used by multiple sockets */
    int optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, sizeof(optval)) < 0) {
        syserr("Reusing ADDR failed");
    }
    /* setting multicast address */
    ip_mreq.imr_multiaddr.s_addr = inet_addr(this->mcast.data());
    ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &ip_mreq, sizeof(ip_mreq)) < 0) {
        syserr("setsockopt");
    }

    /* block multicast to yourself */
    optval = 0;
    if (setsockopt(sock, SOL_IP, IP_MULTICAST_LOOP, (void *) &optval, sizeof optval) < 0) {
        syserr("setsockopt loop");
    }
}

int Connection::openTCPSocket() {
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        syserr("socket");
    }

    /* bind */
    struct sockaddr_in local_address{};
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(0);
    if (bind(sock, (struct sockaddr *) &local_address, sizeof local_address) < 0) {
        syserr("bind");
    }

    /* set to non-blocking */
    int flags;
    if ((flags = fcntl(sock, F_GETFL, 0)) == -1) {
        syserr("fcntl");
    }
    flags = flags & ~O_NONBLOCK;
    if (fcntl(sock, F_SETFL, flags) != 0) {
        syserr("fcntl");
    }

    if (listen(sock, 10) < 0) {
        syserr("listen");
    }
    /* save port and IP address */
    this->TCPport = getPort(sock);
    return sock;
}

int Connection::openTCPSocket(struct sockaddr_in provider) {
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        syserr("socket");
    }

    provider.sin_family = AF_INET;

    /* set to non-blocking */
    int flags;
    if ((flags = fcntl(sock, F_GETFL, 0)) == -1) {
        syserr("fcntl");
    }
    flags = flags & ~O_NONBLOCK;
    if (fcntl(sock, F_SETFL, flags) != 0) {
        syserr("fcntl");
    }

    if (connect(sock, (struct sockaddr *) &provider, sizeof(provider)) == -1) {
        throw NetstoreException("Cannot connect to the server's socket.");
    }
    return sock;
}

void Connection::setReceiver() {
    auto remote_dotted_address = this->mcast.data();
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(this->port);
    if (inet_aton(remote_dotted_address, &remote_address.sin_addr) == 0) {
        syserr("inet_aton");
    }
}

void Connection::setTimeout(int sock) {
    struct timeval tv{};
    tv.tv_sec = ttl;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof tv);
}

void Connection::multicast(int sock, const std::shared_ptr<Message> &message) {
    auto data = message->getRawData();
    if (sendto(sock, data, message->getSize(), 0, (struct sockaddr *) &remote_address,
               sizeof(remote_address)) < 0) {
        syserr("sendto");
    }
    free(data);
}

ConnectionResponse Connection::readFromUDPSocket(int sock) {
    ConnectionResponse response{};
    std::vector<my_byte> buffer(Netstore::MAX_UDP_PACKET_SIZE, 0);
    socklen_t len = sizeof(response.getCliaddr());

    ssize_t singleLen = recvfrom(sock, buffer.data(), Netstore::MAX_UDP_PACKET_SIZE, 0,
                                 (struct sockaddr *) &response.getCliaddr(), &len);
    if (singleLen < 0) {
        throw NetstoreException("Timeout while waiting for connection.");
    }
    response.setBuffer(buffer);
    response.setSize(singleLen);
    return response;
}


int Connection::receiveFile(int sock, FILE *file) {
    std::vector<my_byte> buffer(Netstore::BUFFER_LEN, 0);
    ssize_t len;
    if ((len = read(sock, buffer.data(), buffer.size())) < 0) {
        throw MessageSendException("There was problem with read, while receiving files from server.\n");
    }
    if (len == 0) {
        return 1;
    }
    fwrite(buffer.data(), sizeof(my_byte), len, file);
    buffer.clear();
    return 0;
}

sockaddr_in Connection::getAddr(int sock) {
    sockaddr_in addr{};
    socklen_t len = sizeof(sockaddr_in);
    getsockname(sock, (sockaddr *)&addr, &len);
    return addr;
}

void Connection::sendToSocket(int sock, struct sockaddr_in address, const std::shared_ptr<Message> &message) {
    auto messageRaw = message->getRawData();
    ssize_t len = message->getSize();
    ssize_t snd_len = sendto(sock, messageRaw, len, 0, (struct sockaddr *) &address, sizeof(address));
    if (snd_len != len) {
        throw PartialSendException();
    }
    free(messageRaw);
}

void Connection::closeSocket(int sock) {
    if (close(sock) < 0) {
        syserr("close");
    }
}
