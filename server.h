#ifndef SIK2_SERVER_H
#define SIK2_SERVER_H

#include "Message.h"
#include <utility>

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
            fds(std::vector<pollfd>(N, {0, POLLIN, 0})),
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

    void handleConnection(int sock);

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

    if (fds[0].revents & POLLIN) {
        fds[0].revents = 0;
        int s = accept(fds[0].fd, NULL, 0);
        /* umieść deskryptor s w tablicy fds */
        for (int i = 1; i < N; ++i) {
            if (fds[i].fd == -1) {
                fds[i].fd = s;
            }
        }
    }

    for (int k = 1; k < N; ++k) {
        if ((fds[k].revents & POLLIN) | POLLERR) {
            fds[k].revents = 0;
            /* obsłuż klienta na gnieździe fds[k].fd */
            this->handleConnection(fds[k].fd);
            /* jeżeli read przekaże 0, usuń pozycję k z fds */

        } else if (timeout.find(fds[k].fd) != timeout.end()) {
            if (float(std::clock() - timeout.at(fds[k].fd)) / CLOCKS_PER_SEC > 10) {
                this->connection->closeSocket(fds[k].fd);
            }
        }
    }
}

void ServerNode::handleConnection(int sock) {
    int type;
    int length = sizeof(int);

    /* check if it's UDP or TCP socket */
    if (getsockopt(sock, SOL_SOCKET, SO_TYPE, &type, (socklen_t *) &length) < 0) {
        syserr("getsockopt");
    }

    if (type == SOCK_STREAM) {
        handleTCPConnection(sock);
    } else {
        handleUDPConnection(sock);
    }
}

void ServerNode::handleTCPConnection(int sock) {
    std::vector<char> data = this->connection->receiveFile(sock);
    // save file
}

void ServerNode::handleUDPConnection(int sock) {
    auto response = this->connection->readFromUDPSocket(sock);
    auto message = MessageBuilder::build(response.getBuffer(), 0);
    auto responseMessage = message->getResponse();

    if (responseMessage->getCmd() == "MY_LIST") {
        std::vector<char> data{};
        for (auto f : files) {
            data.insert(data.end(), f.begin(), f.end());
            data.push_back('\n');
        }
        responseMessage->setData(data);
    } else if (responseMessage->getCmd() == "GOOD_DAY") {
        responseMessage->completeMessage(memory,
                                         std::vector<char>(connection->getMcast().begin(),
                                                           connection->getMcast().end()));
    } else if (responseMessage->isOpeningTCP()) {
        int tcp = connection->openTCPSocket();
        fds.push_back({tcp, POLLIN, 0});
        timeout.insert({tcp, std::clock()});
        // save file name with socket
        responseMessage->completeMessage(Connection::getPort(tcp), message->getData());
    }
    connection->sendToSocket(sock, response.getCliaddr(), responseMessage->getRawData());
}

void ServerNode::closeConnections() {
    for (auto fd : fds) {
        connection->closeSocket(fd.fd);
    }
}


#endif //SIK2_SERVER_H
