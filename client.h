#ifndef SIK2_CLIENT_H

#define SIK2_CLIENT_H
#include <wait.h>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <experimental/filesystem>
#include <queue>

#include "Connection.h"

namespace fs = std::experimental::filesystem;


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
            {"discover", [=](const std::string &s) { (void)s; this->discover(); }},
            {"search",   [=](const std::string &s) { this->search(s); }},
            {"fetch",    [=](const std::string &s) { this->fetch(s); }}, // non-blocking
            {"upload",   [=](const std::string &s) { this->upload(s); }}, // non-blocking
            {"remove",   [=](const std::string &s) { this->remove(s); }},
            {"exit",     [=](const std::string &s) { (void)s; this->exit(); }}
    };
    std::shared_ptr<Connection> connection;
    int sock{};
    std::unordered_map<std::string, std::queue<sockaddr_in>> files{};
    std::map<uint64_t , std::queue<sockaddr_in>> memory{};

    std::string folder;
    struct pollfd fds[N]{-1, POLLIN, 0};
    std::unordered_map<int, std::string> clientRequests;

public:
    ClientNode(const std::string &mcast, unsigned port, unsigned int timeout, std::string folder) :
            connection(std::make_shared<Connection>(mcast, port, timeout)),
            folder(std::move(folder)) {}

    void addFile(const std::string &filename, sockaddr_in addr);

    void readUserInput();

    void listen();

    void startConnection();

};
#endif //SIK2_CLIENT_H
