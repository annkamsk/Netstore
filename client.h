#include <utility>

#ifndef SIK2_CLIENT_H
#define SIK2_CLIENT_H

#include "Message.h"
#include <wait.h>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <queue>

namespace fs = std::filesystem;

class ClientNode {
private:
    void discover();

    void search(const std::string &s);

    void fetch(const std::string &s);

    void upload(const std::string &s);

    void remove(const std::string &s);

    void exit();

    static const int N = 20;
    std::map<std::string, std::function<void(std::string)>> commands = {
            {"discover", [=](const std::string &s) { this->discover(); }},
            {"search",   [=](const std::string &s) { this->search(s); }},
            {"fetch",    [=](const std::string &s) { this->fetch(s); }}, // non-blocking
            {"upload",   [=](const std::string &s) { this->upload(s); }}, // non-blocking
            {"remove",   [=](const std::string &s) { this->remove(s); }},
            {"exit",     [=](const std::string &s) { this->exit(); }}
    };
    std::shared_ptr<Connection> connection;
    int sock;

    std::string folder;
    struct pollfd fds[N]{-1, POLLIN, 0};

public:
    ClientNode(const std::string &mcast, unsigned port, unsigned int timeout, std::string folder) :
            connection(std::make_shared<Connection>(mcast, port, timeout)), folder(std::move(folder)) {}

    void addFile(const std::string &filename, sockaddr_in addr);

    void fetchFile(ComplexMessage message);

    void readUserInput();

    void startConnection();

};
#endif //SIK2_CLIENT_H
