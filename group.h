#include <utility>

#include <utility>


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
#include <functional>

#include "err.h"

#define BSIZE 1024

using std::string;
using std::vector;

namespace Netstore {
    const static int MIN_SIZE = 18;
}

class Group {
    string MCAST_ADDR;
    unsigned int CMD_PORT{};

public:
    Group() = default;
    Group(string mcast, unsigned int port) : MCAST_ADDR(std::move(mcast)), CMD_PORT(port) {}
    string getMCAST_ADDR() { return this->MCAST_ADDR; }
    unsigned int getCMD_PORT() { return this->CMD_PORT; }
};

class Connection {
    string ip;
    int sock{};
    int port{};
    struct ip_mreq ip_mreq{};


public:
    Connection() = default;
    Connection(string ip, int port) : ip(std::move(ip)), port(port){}
    void openSocket();
    void addServerNodeToMcast(string mcast);
    void addToLocal();
    void detachFromGroup();
    void closeSocket();

    vector<char> readFromSocket(unsigned int expectedLen);
};

class Command {
    string cmd;
    long cmd_seq{}; // bigendian
    static long getSeq() {
        return 0;
    }

public:
    explicit Command(string cmd) : cmd(std::move(cmd)), cmd_seq(getSeq()) {}
    explicit Command(vector<char> data) {
        data.erase(remove_if(data.begin(), data.begin() + 10, [](char c) { return !isalpha(c); } ),
                data.begin() + 10);
        cmd = data.data();
    }
    bool isGreet() { return !cmd.compare("HELLO"); }
    bool isList() { return !cmd.compare("LIST"); }
    bool isGetFile() { return !cmd.compare("GET"); }
    bool isDeleteFile() { return !cmd.compare("DEL"); }
    bool isAddFile() { return !cmd.compare("ADD"); }
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
    Connection connection;

public:
    Node() = default;
    Node(string ip, int port, Group group) : group(std::move(group)), connection(Connection(std::move(ip), port)) {}
    explicit Node(Group group) : group(std::move(group)), connection(Connection()) {}

    void detachFromGroup();
    void closeSocket();
    void addToMcast();
    void openSocket();
    void addToLocal();

    virtual void greet(Command command) = 0;
    virtual void getList(Command command) = 0;
    virtual void getListWith(Command command) = 0;
    virtual char getFile(Command command) = 0;
    virtual void deleteFile(Command command) = 0;
    virtual void addFile(char data[]) = 0;
};


#endif //SIK2_GROUP_H
