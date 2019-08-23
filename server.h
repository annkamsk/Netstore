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
    uint64_t memory{};
public:

    ServerNode(const std::string &mcast, unsigned port, unsigned long long memory, unsigned int timeout,
               std::string folder) :
            folder(std::move(folder)),
            connection(std::make_shared<Connection>(mcast, port, timeout)),
            fds(std::vector<pollfd>(N, {-1, POLLIN, 0})),
            memory(memory){};

    unsigned long long getMemory() {
        return memory;
    }

    const std::string &getFolder() const {
        return folder;
    }

    void addFile(const std::string &filename, uint64_t size) {
        this->files.push_back(filename);
        this->memory -= size;
    }

    void createConnection();

    void listen();

    void handleUDPConnection(int sock);

    void handleTCPConnection(int sock);

    void closeConnections();
};

void ServerNode::createConnection() {
    int sock = connection->openUDPSocket();
    connection->addToMulticast(sock);
    fds.insert(fds.begin(), {sock, POLLIN, 0});
}

void ServerNode::listen() {
    poll(&fds[0], N, -1);

    /* handle event on a UDP socket */
    if (fds[0].revents & POLLIN) {
        fds[0].revents = 0;
        this->handleUDPConnection(fds[0].fd);
    }

    for (int k = 1; k < N; ++k) {
        if ((fds[k].revents & POLLIN) | POLLERR) {
            fds[k].revents = 0;
            /* obsłuż klienta na gnieździe fds[k].fd */
            this->handleTCPConnection(fds[k].fd);
            /* jeżeli read przekaże 0, usuń pozycję k z fds */

        } else if (timeout.find(fds[k].fd) != timeout.end()) {
            if (float(std::clock() - timeout.at(fds[k].fd)) / CLOCKS_PER_SEC > 10) {
                this->connection->closeSocket(fds[k].fd);
            }
        }
    }
}

void ServerNode::handleTCPConnection(int sock) {
    std::vector<char> data = this->connection->receiveFile(sock);
    // save file
}

void ServerNode::handleUDPConnection(int sock) {
    auto request = this->connection->readFromUDPSocket(sock);
    auto message = MessageBuilder::build(request.getBuffer(), 0);
    auto response = message->getResponse();

    if (response->getCmd() == "MY_LIST") {
        std::vector<char> data{};
        for (auto f : files) {
            data.insert(data.end(), f.begin(), f.end());
            data.push_back('\n');
        }
        response->setData(data);
    } else if (response->getCmd() == "GOOD_DAY") {
        response->completeMessage(memory,
                                         std::vector<char>(connection->getMcast().begin(),
                                                           connection->getMcast().end()));
    } else if (response->isOpeningTCP()) {
        int tcp = connection->openTCPSocket();
        fds.push_back({tcp, POLLIN, 0});
        timeout.insert({tcp, std::clock()});
        // save file name with socket
        response->completeMessage(Connection::getPort(tcp), message->getData());
    }
    connection->sendToSocket(sock, request.getCliaddr(), response);
}

void ServerNode::closeConnections() {
    for (auto fd : fds) {
        connection->closeSocket(fd.fd);
    }
}


#endif //SIK2_SERVER_H
