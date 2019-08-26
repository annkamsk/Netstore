#ifndef SIK2_SERVER_H
#define SIK2_SERVER_H

#include "Connection.h"

class ServerNode {

    const static int N = 256;
private:
    std::vector<std::string> files;
    std::string folder;
    std::shared_ptr<Connection> connection;
    std::vector<pollfd> fds;
    std::unordered_map<int, clock_t> timeout;
    std::unordered_map<int, std::string> fileToSend;
    uint64_t memory{};
public:

    ServerNode(const std::string &mcast, unsigned port, unsigned long long memory, unsigned int timeout,
               std::string folder) :
            folder(std::move(folder)),
            connection(std::make_shared<Connection>(mcast, port, timeout)),
            fds(std::vector<pollfd>(N, {-1, POLLIN, 0})),
            memory(memory) {};

    void addFile(const std::string &filename, uint64_t size) {
        this->files.push_back(filename);
        this->memory -= size;
    }

    void createConnection();

    void listen();

    void handleUDPConnection(int sock);

    void handleTCPConnection(int sock);

    void closeConnections();

    std::vector<my_byte> getFiles(const std::vector<my_byte> &s);

    void sendList(int sock, const ConnectionResponse& request, const std::shared_ptr<Message>& message);

    void sendGreeting(int sock, const ConnectionResponse& request, const std::shared_ptr<Message>& message);

    void prepareDownload(int sock, const ConnectionResponse& request, const std::shared_ptr<Message>& message);

    void prepareUpload(int sock, const ConnectionResponse& request, const std::shared_ptr<Message>& message);

    void deleteFiles(const std::vector<my_byte>& data);

    int openToClient(const std::string& filename);

    bool isUploadValid(const std::string&, uint64_t size);

    unsigned long long getMemory() {
        return memory;
    }

    const std::string &getFolder() const {
        return folder;
    }
};

#endif //SIK2_SERVER_H
