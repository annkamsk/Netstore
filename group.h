
#ifndef SIK2_GROUP_H
#define SIK2_GROUP_H

#include <string>
#include <vector>
#include <list>
#include <utility>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>

#include "err.h"

using std::string;
using std::vector;

class Group {
    string MCAST_ADDR;
    unsigned int CMD_PORT{};
    int sock;

public:
    Group() = default;
    Group(string mcast, unsigned int port) : MCAST_ADDR(std::move(mcast)), CMD_PORT(port) {}

    void openSocket();
    void addServerNodeToMcast();

};

class Command {
    string cmd;
    long cmd_seq; // bigendian

    static long getSeq() {
        return 0;
    }

public:
    Command(string cmd) : cmd(std::move(cmd)), cmd_seq(getSeq()) {}
};

class BasicCommand : Command {
    char data[];
};

class ComplexCommand : Command {
    long param;
    char data[];
};

class Node {
protected:
    Group group;
    string ip;
    int port;

public:
    Node() = default;
    Node(string ip, int port, Group group) : group(std::move(group)), ip(std::move(ip)), port(port)  {}
    Node(string ip, int port) : group(Group()), ip(std::move(ip)), port(port) {}
    Node(Group group) : group(std::move(group)) {};

    virtual void greet() = 0;
    virtual std::list<string> getList() = 0;
    virtual std::list<string> getListWith(string pattern) = 0;
    virtual char getFile(string name) = 0;
    virtual void deleteFile(string name) = 0;
    virtual void addFile(char data[]) = 0;
};


#endif //SIK2_GROUP_H
