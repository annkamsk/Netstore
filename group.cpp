
#include <iostream>
#include "group.h"

void Group::addServerNodeToMcast() {
    struct ip_mreq ip_mreq{};
    char *multicast_dotted_address = this->MCAST_ADDR.data();
        ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (inet_aton(multicast_dotted_address, &ip_mreq.imr_multiaddr) == 0) {
        syserr("inet_aton");
    }
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&ip_mreq, sizeof ip_mreq) < 0) {
        syserr("setsockopt");
    }
}

void Group::openSocket() {
    if ((this->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        syserr("socket");
    }
}