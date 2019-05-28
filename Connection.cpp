#include "Connection.h"

void Connection::addToLocal() {
    struct sockaddr_in local_address{};
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(this->port);
    if (bind(this->sock, (struct sockaddr *) &local_address, sizeof local_address) < 0) {
        syserr("bind");
    }
}

void Connection::addToMcast() {
    /* setting multicast address */
    char *multicast_dotted_address = this->mcast.data();
    ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (inet_aton(multicast_dotted_address, &ip_mreq.imr_multiaddr) == 0) {
        syserr("inet_aton");
    }
    if (setsockopt(this->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &this->ip_mreq, sizeof this->ip_mreq) < 0) {
        syserr("setsockopt");
    }

    /* set TTL */
    uint optval = this->ttl;
    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval, sizeof(optval));

    /* block multicast to yourself */
    optval = 0;
    if (setsockopt(sock, SOL_IP, IP_MULTICAST_LOOP, (void *) &optval, sizeof optval) < 0) {
        syserr("setsockopt loop");
    }
}

void Connection::detachFromGroup() {
    if (setsockopt(this->sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *) &this->ip_mreq, sizeof this->ip_mreq) < 0) {
        syserr("setsockopt");
    }
}

void Connection::closeSocket() {
    close(this->sock);
}

ConnectionResponse UDPConnection::readFromSocket() {
    ConnectionResponse response{};
    std::vector<char> buffer(Netstore::MAX_SMPL_CMD_SIZE);
    socklen_t len;
    ssize_t singleLen = recvfrom(this->sock, &buffer[0], buffer.size(), 0, (struct sockaddr *) &response.getCliaddr(),
                                 &len);

    if (singleLen < 0) {
        syserr("read");
    } else {
        printf("read %zd bytes: %.*s\n", singleLen, (int) singleLen, buffer.data());
    }
    response.setBuffer(buffer);
    return response;
}

void UDPConnection::sendToSocket(struct sockaddr_in address, std::string data) {
    ssize_t len = data.size();

    ssize_t snd_len = sendto(this->sock, data.data(), len, 0, (struct sockaddr *) &address, sizeof address);
    if (snd_len != len) {
        throw PartialSendException();
    }
}

void UDPConnection::openSocket() {
    if ((this->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        syserr("socket");
    }
}

void TCPConnection::openSocket() {
    if ((this->sock = socket(AF_INET, SOCK_PACKET, 0)) < 0) {
        syserr("socket");
    }
}

ConnectionResponse TCPConnection::readFromSocket() {
    return ConnectionResponse();
}
