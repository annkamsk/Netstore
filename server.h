#ifndef SIK2_SERVER_H
#define SIK2_SERVER_H

#include "group.h"
#include <utility>

class ServerNode : public Node {

private:
    uint64_t memory{};

public:
    ServerNode(const std::string &mcast, unsigned port, unsigned long long memory, unsigned int timeout,
               const std::string &folder) :
            Node(mcast, port, timeout, folder), memory(memory) {};

    unsigned long long getMemory() override {
        return memory;
    }

    void addFile(const std::string &filename, unsigned long long size) override;

    std::shared_ptr<Connection> startConnection() override;
};

#endif //SIK2_SERVER_H
