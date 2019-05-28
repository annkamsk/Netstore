
#ifndef SIK2_GROUP_H
#define SIK2_GROUP_H

#include "Connection.h"

using std::string;
using std::vector;

class Group {
    string MCAST_ADDR;
    unsigned int CMD_PORT{};

public:
    Group() = default;

    Group(string mcast, unsigned int port) : MCAST_ADDR(std::move(mcast)), CMD_PORT(port) {}

    string getMCAST_ADDR() { return this->MCAST_ADDR; }

    unsigned int getCMD_PORT() { return this->CMD_PORT; }
};


class Node {
protected:
    Group group;

public:
    Node() = default;

    explicit Node(Group group) : group(std::move(group)) {}

    virtual std::shared_ptr<Connection> startConnection() {
        return std::make_shared<UDPConnection>(UDPConnection(group.getMCAST_ADDR(), group.getCMD_PORT(), 0));
    }

    Group getGroup() {
        return group;
    }

    virtual unsigned long long getMemory() {
        return 0;
    }

    virtual const vector<string> & getFiles() const { return vector<std::string>{}; }
};


#endif //SIK2_GROUP_H
