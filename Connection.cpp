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
    /* setting multicast address */
    char *multicast_dotted_address = this->mcast.data();
    ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (inet_aton(multicast_dotted_address, &ip_mreq.imr_multiaddr) == 0) {
        syserr("inet_aton");
    }
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &this->ip_mreq, sizeof this->ip_mreq) < 0) {
        syserr("setsockopt");
    }

    /* block multicast to yourself */
    int optval = 0;
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
    if (listen(sock, 10) < 0) {
        syserr("listen");
    }
    return sock;
}

void Connection::setReceiver() {
    char *remote_dotted_address = this->mcast.data();

    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(this->port);
    if (inet_aton(remote_dotted_address, &remote_address.sin_addr) == 0) {
        syserr("inet_aton");
    }
}

void Connection::multicast(int sock, std::string data) {
    if (sendto(sock, data.data(), data.size(), 0, (struct sockaddr*) &remote_address, sizeof(remote_address)) < 0) {
        syserr("sendto");
    }
}

ConnectionResponse Connection::readFromUDPSocket(int sock) {
    ConnectionResponse response{};
    std::vector<char> buffer(Netstore::MAX_SMPL_CMD_SIZE);
    socklen_t len;
    std::cout << "reading";
    ssize_t singleLen = recvfrom(sock, &buffer[0], buffer.size(), 0, (struct sockaddr *) &response.getCliaddr(), &len);

    if (singleLen < 0) {
        syserr("read");
    } else {
        std::cout << "read " << singleLen << " bytes: " << buffer.data() << "\n";
    }
    response.setBuffer(buffer);
    return response;
}

std::vector<char> Connection::receiveFile(int sock) {
    std::vector<char> data;
    std::vector<char> buffer(Netstore::BUFFER_LEN);
    ssize_t len;
    do {
        if ((len = read(sock, &buffer[0], buffer.size())) < 0) {
            throw MessageSendException("There was problem with read, while receiving files from server.\n");
        }
        std::cout << "Read " << len << " bytes from socket.\n";
        data.insert(data.end(), buffer.begin(), buffer.begin() + len);
        buffer.clear();
    } while (len > 0);
    return data;
}


void Connection::sendToSocket(int sock, struct sockaddr_in address, std::string data) {
    ssize_t len = data.size();

    ssize_t snd_len = sendto(sock, data.data(), len, 0, (struct sockaddr *) &address, sizeof address);
    if (snd_len != len) {
        throw PartialSendException();
    }
    std::cout << "Sent " << data.length() << " bytes of data: " << data.data() << " to "
              << inet_ntoa(address.sin_addr) << "\n";
}

void Connection::closeSocket(int sock) {
    if (close(sock) < 0) {
        syserr("close");
    }
}

