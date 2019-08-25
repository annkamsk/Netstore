#include "server.h"

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
        if (fds[k].fd != -1 && ((fds[k].revents & POLLIN) | POLLERR)) {
            fds[k].revents = 0;
            /* obsłuż klienta na gnieździe fds[k].fd */
            this->handleTCPConnection(fds[k].fd);
            /* jeżeli read przekaże 0, usuń pozycję k z fds */

        } else if (timeout.find(fds[k].fd) != timeout.end()) {
            if (float(clock() - timeout.at(fds[k].fd)) / CLOCKS_PER_SEC > 10) {
                this->connection->closeSocket(fds[k].fd);
            }
        }
    }
}

void ServerNode::handleTCPConnection(int sock) {
    std::vector<my_byte> data = this->connection->receiveFile(sock);
    // save file
}

void ServerNode::handleUDPConnection(int sock) {
    auto request = this->connection->readFromUDPSocket(sock);
    auto message = MessageBuilder::build(request.getBuffer(), 0, request.getSize());
    auto response = message->getResponse();

    if (message->getCmd() == "LIST") {
        sendList(sock, request, message);
    } else if (message->getCmd() == "HELLO") {
        sendGreeting(sock, request, message);
    } else if (response->isOpeningTCP()) {
        int tcp = connection->openTCPSocket();
        fds.push_back({tcp, POLLIN, 0});
        timeout.insert({tcp, clock()});
        // save file name with socket
        response->completeMessage(Connection::getPort(tcp), message->getData());
    } else if (message->getCmd() == "DEL") {
        deleteFiles(message->getData());
    } else {
        connection->sendToSocket(sock, request.getCliaddr(), response);
    }
}

void ServerNode::closeConnections() {
    for (auto fd : fds) {
        connection->closeSocket(fd.fd);
    }
}

std::vector<my_byte> ServerNode::getFiles(const std::vector<my_byte> &s) {
    std::string searched(s.begin(), s.end());
    std::vector<my_byte> data{};
    for (auto f : files) {
        /* if the filename contains given substring */
        if (s.empty() || f.find(searched) != std::string::npos) {
            data.insert(data.end(), f.begin(), f.end());
            data.push_back('\n');
        }
    }
    return data;
}


void ServerNode::sendList(int sock, const ConnectionResponse& request, const std::shared_ptr<Message>& message) {
    auto response = message->getResponse();
    auto foundFiles = getFiles(message->getData());
    if (!foundFiles.empty()) {
        response->setData(getFiles(message->getData()));
        // TODO when list greater than max UDP packet size
        connection->sendToSocket(sock, request.getCliaddr(), response);
    }
}

void ServerNode::sendGreeting(int sock, const ConnectionResponse& request, const std::shared_ptr<Message>& message) {
    auto response = message->getResponse();
    response->completeMessage(memory,
                              std::vector<my_byte>(connection->getMcast().begin(), connection->getMcast().end()));
    connection->sendToSocket(sock, request.getCliaddr(), response);
}

void ServerNode::deleteFiles(const std::vector<my_byte>& data) {
    size_t len = files.size();
    std::string name(data.begin(), data.end());

    /* remove file with the specified name */
    files.erase(std::remove(files.begin(), files.end(), name), files.end());

    /* if there were files with this name, delete them from disc */
    if (len != files.size()) {
        std::string path = folder + "/" + name;
        if (remove(path.data()) != 0) {
            std::cerr << "Error deleting file: No such file or directory\n";
        } else {
            std::cerr << "File " << name << " deleted.\n";

        }
    }
}
