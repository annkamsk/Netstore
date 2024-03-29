#ifndef SIK2_CLIENT_H

#define SIK2_CLIENT_H
#include <wait.h>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <experimental/filesystem>
#include <queue>

#include "Connection.h"
#include "FileSender.h"

namespace fs = std::experimental::filesystem;


class ClientNode {

    struct ClientRequest {
        std::string filename;
        sockaddr_in server;
        FILE *f;
    };
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
    std::string folder;
    std::shared_ptr<Connection> connection;
    int sock{};
    std::vector<pollfd> fds;
    std::vector<FileSender> uploadFiles;

    std::unordered_map<std::string, std::vector<sockaddr_in>> files{};
    std::map<uint64_t, std::vector<sockaddr_in>, std::greater<uint64_t >> memory{};
    std::unordered_map<int, ClientRequest> downloadRequests;

    MessageBuilder messageBuilder;

public:
    ClientNode(const std::string &mcast, unsigned port, unsigned int timeout, std::string folder) :
            folder(std::move(folder)),
            connection(std::make_shared<Connection>(mcast, port, timeout)),
            fds(std::vector<pollfd>(N, {-1, POLLIN, 0})),
            uploadFiles(std::vector<FileSender>()),
            messageBuilder(MessageBuilder()) {}

    void addFile(const std::string &filename, sockaddr_in addr);

    void readUserInput();

    void listen();

    void startConnection();

    void handleFileReceiving(int sock);

    void handleFileDownloaded(int fd);

    void handleFileSending();

    int connectWithServer(uint64_t param, sockaddr_in &addr);

    sockaddr_in findServer(const std::string &s, size_t size);

    bool hasFile(sockaddr_in server, const std::string& file);
};
#endif //SIK2_CLIENT_H
