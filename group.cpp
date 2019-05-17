
#include <iostream>
#include "group.h"

void Connection::openSocket() {
    if ((this->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        syserr("socket");
    }
}

void Connection::addToLocal() {
    struct sockaddr_in local_address{};
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(this->port);
    if (bind(this->sock, (struct sockaddr *)&local_address, sizeof local_address) < 0) {
        syserr("bind");
    }
}

void Connection::addServerNodeToMcast(string mcast) {
    char *multicast_dotted_address = mcast.data();
    ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (inet_aton(multicast_dotted_address, &ip_mreq.imr_multiaddr) == 0) {
        syserr("inet_aton");
    }
    if (setsockopt(this->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&this->ip_mreq, sizeof this->ip_mreq) < 0) {
        syserr("setsockopt");
    }
}

void Connection::detachFromGroup() {
    if (setsockopt(this->sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&this->ip_mreq, sizeof this->ip_mreq) < 0) {
        syserr("setsockopt");
    }
}

void Connection::closeSocket() {
    close(this->sock);
}

void Node::detachFromGroup() {
    this->connection.detachFromGroup();
}

void Node::closeSocket() {
    this->connection.closeSocket();
}

void Node::addToMcast() {
    this->connection.addServerNodeToMcast(this->group.getMCAST_ADDR());
}

void Node::openSocket() {
    this->connection.openSocket();
}

void Node::addToLocal() {
    this->connection.addToLocal();
}

