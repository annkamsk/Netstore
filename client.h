#ifndef SIK2_CLIENT_H
#define SIK2_CLIENT_H

#include "group.h"
#include "Message.h"
#include <wait.h>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <queue>

namespace fs = std::filesystem;

class ClientNode : public Node {
private:
    void discover();

    void search(const std::string &s);

    void fetch(const std::string &s);

    void upload(const std::string &s);

    void remove(const std::string &s);

    void exit();

    std::map<std::string, std::function<void(std::string)>> commands = {
            {"discover", [=](const std::string &s) { this->discover(); }},
            {"search",   [=](const std::string &s) { this->search(s); }},
            {"fetch",    [=](const std::string &s) { this->fetch(s); }},
            {"upload",   [=](const std::string &s) { this->upload(s); }},
            {"remove",   [=](const std::string &s) { this->remove(s); }},
            {"exit",     [=](const std::string &s) { this->exit(); }}
    };
    std::map<std::string, std::queue<sockaddr_in>> files;
    std::map<uint64_t, std::queue<sockaddr_in>> space;
    std::map<int, uint64_t> fdToSeq;
    std::map<int, sockaddr_in> fdToAddr;
    static const int N = 20;
    struct pollfd fds[N]{-1, POLLIN, 0};
    int ite{};


public:

    ClientNode(const std::string &mcast, unsigned port, unsigned int timeout, const std::string &folder) :
            Node(mcast, port, timeout, folder) {}

    void readInput();

    void addFile(const std::string &filename, sockaddr_in addr) override;

    void fetchFile(ComplexGetMessage message);

    void startUserInput();

    void readUserInput();

    std::shared_ptr<Connection> startConnection();

    void addTCPConnection();
};
#endif //SIK2_CLIENT_H
