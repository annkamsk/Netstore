#include <asm/ioctls.h>
#include <stropts.h>
#include <bits/fcntl-linux.h>
#include <fcntl.h>
#include "Connection.h"

void Connection::addToLocal(unsigned portt) {
    struct sockaddr_in local_address{};
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(portt);
    if (bind(this->masterSock, (struct sockaddr *) &local_address, sizeof local_address) < 0) {
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
    if (setsockopt(this->masterSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &this->ip_mreq, sizeof this->ip_mreq) <
        0) {
        syserr("setsockopt");
    }

    /* set TTL */
    uint optval = this->ttl;
    setsockopt(this->masterSock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval, sizeof(optval));

    /* block multicast to yourself */
//    optval = 0;
//    if (setsockopt(this->masterSock, SOL_IP, IP_MULTICAST_LOOP, (void *) &optval, sizeof optval) < 0) {
//        syserr("setsockopt loop");
//    }
}

void Connection::detachFromGroup() {
    if (setsockopt(this->masterSock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *) &this->ip_mreq, sizeof this->ip_mreq) <
        0) {
        syserr("setsockopt");
    }
}

void Connection::closeSocket() {
    close(this->getSock());
}

void Connection::activateBroadcast() {
    int optval = 1;
    if (setsockopt(this->masterSock, SOL_SOCKET, SO_BROADCAST, (void *) &optval, sizeof optval) < 0) {
        syserr("setsockopt broadcast");
    }

    /* set TTL */
    optval = this->ttl;
    if (setsockopt(this->masterSock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval, sizeof optval) < 0) {
        syserr("setsockopt multicast ttl");
    }

    /* set local TTL */
    optval = this->ttl;
    if (setsockopt(this->masterSock, IPPROTO_IP, IP_TTL, (void *) &optval, sizeof optval) < 0) {
        syserr("setsockopt ttl");
    }

    /* block broadcasting to yourself */
//    optval = 0;
//    if (setsockopt(this->masterSock, SOL_IP, IP_MULTICAST_LOOP, (void*)&optval, sizeof optval) < 0) {
//        syserr("setsockopt loop");
//    }

}

void Connection::setReceiver() {
    char *remote_dotted_address = this->mcast.data();

    struct sockaddr_in remote_address{};
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(this->port);
    if (inet_aton(remote_dotted_address, &remote_address.sin_addr) == 0) {
        syserr("inet_aton");
    }
    if (connect(this->masterSock, (struct sockaddr *) &remote_address, sizeof remote_address) < 0) {
        syserr("connect");
    }
}

void Connection::broadcast(std::string data) {

}

ConnectionResponse Connection::waitForResponse() {
    timeval tv{this->ttl, 0};
    fd_set readfds;
    FD_SET(this->masterSock, &readfds);
    int nready;
    if ((nready = select(this->masterSock + 1, &readfds, nullptr, nullptr, &tv)) < 0) {
        syserr("select error");
    }
    std::cout << "Incoming " << nready << " messages.\n";
    if (nready > 0) {
        auto response = readFromSocket();
        return response;
    }
    return ConnectionResponse();
}

ConnectionResponse UDPConnection::readFromSocket() {
    ConnectionResponse response{};
    std::vector<char> buffer(Netstore::MAX_SMPL_CMD_SIZE);
    socklen_t len;
    std::cout << "listening";
    ssize_t singleLen = recvfrom(this->masterSock, &buffer[0], buffer.size(), 0,
                                 (struct sockaddr *) &response.getCliaddr(),
                                 &len);

    if (singleLen < 0) {
        syserr("read");
    } else {
        std::cout << "read " << singleLen << " bytes: " << buffer.data() << "\n";
    }
    response.setBuffer(buffer);
    return response;
}


void UDPConnection::sendToSocket(struct sockaddr_in address, std::string data) {
    ssize_t len = data.size();

    ssize_t snd_len = sendto(this->masterSock, data.data(), len, 0, (struct sockaddr *) &address, sizeof address);
    if (snd_len != len) {
        throw PartialSendException();
    }
    std::cout << "Sent " << data.length() << " bytes of data: " << data.data() << " to "
              << inet_ntoa(address.sin_addr) << "\n";
}

void UDPConnection::openSocket() {
    if ((this->masterSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        syserr("socket");
    }
}

void UDPConnection::broadcast(std::string data) {
    if (write(this->masterSock, data.data(), data.length()) != data.length()) {
        syserr("write");
    }
    std::cout << "Sent " << data.length() << " bytes of data " << data + "\n";
}

void TCPConnection::openSocket() {
    if ((fds[0].fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        syserr("socket");
    }
    /* allow socket descriptor to be reusable */
    int on = 1;
    if (setsockopt(fds[0].fd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) < 0) {
        close(fds[0].fd);
        syserr("setsockopt");
    }

    /* set socket to be non blocking */
    if (fcntl(fds[0].fd, F_SETFL, O_NONBLOCK) < 0) {
        close(fds[0].fd);
        syserr("ioctl");
    }
}

ConnectionResponse TCPConnection::readFromSocket() {
    do {
        int res;
        if ((res = poll(fds, N, this->ttl)) < 0) {
            syserr("poll");
            break;
        }
        if (res == 0) {
            std::cout << "Poll timed out.\n";
            break;
        }
        if (fds[0].revents & POLLIN) {
            fds[0].revents = 0;
            int s = accept(fds[0].fd, nullptr, nullptr);

            /* save descriptor in fds */
            for (ssize_t k = 1; k < N; ++k) {
                if (fds[k].fd == -1) {
                    fds[k].fd = s;
                    fds[k].revents = 1;
                    break;
                }
            }
        }
        for (ssize_t k = 1; k < N; ++k) {
            if ((fds[k].revents & POLLIN) | POLLERR ) {
                fds[k].revents = 0;

//                if (read(fds[k].fd)) {

                    /* obsłuż klienta na gnieździe fds[k].fd */
                    /* jeżeli read przekaże 0, usuń pozycję k z fds */
//                }

            }
        }
        return ConnectionResponse();
    } while(1);
}

void TCPConnection::addToLocal(unsigned port) {
    struct sockaddr_in local_address{};
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(port);
    if (bind(this->fds[0].fd, (struct sockaddr *) &local_address, sizeof local_address) < 0) {
        syserr("bind");
    }
}

void TCPConnection::setToListen() {
    if (listen(fds[0].fd, 32) < 0) {
        close(fds[0].fd);
        exit(-1);
    }
}
