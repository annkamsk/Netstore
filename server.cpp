#include "server.h"

void ServerNode::createConnection() {
    int sock = connection->openUDPSocket();
    connection->addToMulticast(sock);
    fds.at(0) = {sock, POLLIN, 0};
    int tcpSock = connection->openTCPSocket();
    fds.at(1) = {tcpSock, POLLIN, 0};
}

void ServerNode::listen() {
    poll(&fds[0], N, 0);

    /* handle event on a UDP socket */
    if (fds[0].revents & POLLIN) {
        fds[0].revents = 0;
        this->handleUDPConnection(fds[0].fd);
    }
    /* handle event on TCP socket */
    if (fds[1].revents & POLLIN) {
        fds[1].revents = 0;
        tryAccept(fds[1].fd);
    }
    for (size_t k = 2; k < fds.size(); ++k) {
        if (fds[k].fd != -1 && (fds[k].revents & POLLIN || fds[k].fd & POLLERR)) {
            fds[k].revents = 0;
            if (this->handleFileReceiving(fds[k].fd)) {
                fds[k].fd = -1;
            }
        }
    }
    auto it = clientRequests.begin();
    while (it != clientRequests.end()) {
        if (!it->second.isActive) {
            if (time(nullptr) - it->second.timeout > this->connection->getTtl()) {
                /* close connection */
                it = clientRequests.erase(it);
            } else {
                tryAccept(fds[1].fd);
            }
        }
        ++it;
    }
    handleFileSending();
}

int ServerNode::handleFileReceiving(int sock) {
    if (filesUploaded.find(sock) != filesUploaded.end()) {
        auto file = filesUploaded.at(sock);
        if (this->connection->receiveFile(sock, file)) {
            if (fclose(file) < 0) {
                throw FileException("Unable to close downloaded file.");
            }
            filesUploaded.erase(sock);
            close(sock);
            return 1;
        }
    }
    return 0;
}

void ServerNode::handleFileSending() {
    auto it = pendingFiles.begin();
    while (it != pendingFiles.end()) {
        auto f = *it;
        auto clientId = Netstore::getKey(f.getClientAddr());
        auto request = clientRequests.at(clientId);
        try {
            if (f.handleSending()) {
                clientRequests.erase(clientId);
                it = pendingFiles.erase(it);
            } else {
                ++it;
            }
        } catch (NetstoreException &e) {
            std::cerr << "File " << request.filename << " uploading failed. Reason: " << e.what() << " " << e.details();
            it->closeConnection();
            it = pendingFiles.erase(it);
            clientRequests.erase(clientId);
        }
    }
}

int ServerNode::tryAccept(int sock) {
    /* accept the connection */
    sockaddr_in clientAddr{};
    socklen_t len = sizeof(clientAddr);
    int newSock;
    if ((newSock = accept(sock, (sockaddr *) &clientAddr, &len)) == -1) {
        return -1;
    }
    /* find a place for a new socket */
    size_t k = 2;
    for (; k < fds.size() && fds[k].fd != -1; ++k) {}
    if (k == fds.size()) {
        close(newSock);
        throw MessageSendException("Cannot accept the new connection now.");
    }
    /* save new file descriptor*/
    fds[k].fd = newSock;
    handleTCPConnection(newSock, clientAddr, k);
    return newSock;
}

void ServerNode::handleTCPConnection(int newSock, sockaddr_in clientAddr, int k) {
    /* see what client requested before */
    std::string clientId = Netstore::getKey(clientAddr);
    if (clientRequests.find(clientId) == clientRequests.end()) {
        std::cerr << "[PCKG ERROR] Skipping invalid package from " << inet_ntoa(clientAddr.sin_addr) << ":"
                  << ntohs(clientAddr.sin_port) << ". Client does not have any requests waiting.";
        throw NetstoreException();
    }
    ClientRequest &clientRequest = clientRequests.at(clientId);
    clientRequests.at(clientId).isActive = true;
    std::string path = folder + "/" + clientRequest.filename;
    if (clientRequest.isToSend) {
        /* initiate sending file */
        FileSender sender{};
        sender.init(path, newSock, clientAddr);
        pendingFiles.push_back(sender);
    } else {
        /* initiate receiving file */
        FILE *f;
        if ((f = fopen(path.data(), "wb")) == nullptr) {
            fds[k].fd = -1;
            close(newSock);
            throw FileException("Unable to open file " + path);
        }
        filesUploaded.insert({newSock, f});
        clientRequests.erase(clientId);
        this->connection->receiveFile(newSock, f);
    }
}

void ServerNode::handleUDPConnection(int sock) {
    auto request = this->connection->readFromUDPSocket(sock);
    auto message = messageBuilder.build(request.getBuffer(), 0, request.getSize());

    if (message->getCmd() == "LIST") {
        sendList(sock, request, message);
    } else if (message->getCmd() == "HELLO") {
        sendGreeting(sock, request, message);
    } else if (message->getCmd() == "GET") {
        prepareDownload(sock, request, message);
    } else if (message->getCmd() == "ADD") {
        prepareUpload(sock, request, message);
    } else if (message->getCmd() == "DEL") {
        deleteFiles(message->getData());
    }
}

void ServerNode::sendList(int sock, const ConnectionResponse &request, const std::shared_ptr<Message> &message) {
    auto response = message->getResponse();
    auto foundFiles = getFiles(message->getData());
    if (!foundFiles.empty()) {
        response->setData(getFiles(message->getData()));
        // TODO when list greater than max UDP packet size
        connection->sendToSocket(sock, request.getCliaddr(), response);
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

void ServerNode::sendGreeting(int sock, const ConnectionResponse &request, const std::shared_ptr<Message> &message) {
    auto response = message->getResponse();
    std::vector<my_byte> addr(connection->getMcast().begin(), connection->getMcast().end());
    response->completeMessage(memory, addr);
    connection->sendToSocket(sock, request.getCliaddr(), response);
}


void ServerNode::prepareDownload(int sock, const ConnectionResponse &request, const std::shared_ptr<Message> &message) {
    std::string filename(message->getData().begin(), message->getData().end());

    if (std::find(files.begin(), files.end(), filename) != files.end()) {
        auto port = (uint64_t) this->connection->getTCPport();
        /* save request */
        auto clientAddr = request.getCliaddr();
        clientRequests.insert({Netstore::getKey(clientAddr), {time(nullptr), filename, true, false}});

        /* send response to client */
        auto response = message->getResponse();
        response->completeMessage(port, std::vector<my_byte>(filename.begin(), filename.end()));
        connection->sendToSocket(sock, request.getCliaddr(), response);
    } else {
        std::cerr << "[PCKG ERROR] Skipping invalid package from " << inet_ntoa(request.getCliaddr().sin_addr) << ":"
                  << ntohs(request.getCliaddr().sin_port) << ". Reason: requested file not found.";
    }
}

void ServerNode::prepareUpload(int sock, const ConnectionResponse &request, const std::shared_ptr<Message> &message) {
    std::string filename(message->getData().begin(), message->getData().end());

    if (isUploadValid(filename, message->getParam())) {
        auto port = (uint64_t) this->connection->getTCPport();
        /* save request */
        auto clientAddr = request.getCliaddr();
        clientRequests.insert({Netstore::getKey(clientAddr), {time(nullptr), filename, false, false}});

        auto response = messageBuilder.create("CAN_ADD");
        response->setSeq(message->getCmdSeq());
        response->completeMessage(port, std::vector<my_byte>(filename.begin(), filename.end()));
        connection->sendToSocket(sock, request.getCliaddr(), response);
    } else {
        auto response = messageBuilder.create("NO_WAY");
        response->setSeq(message->getCmdSeq());
        response->setData(std::vector<my_byte>(filename.begin(), filename.end()));
        connection->sendToSocket(sock, request.getCliaddr(), response);
    }
}

void ServerNode::deleteFiles(const std::vector<my_byte> &data) {
    size_t len = files.size();
    std::string name(data.begin(), data.end());

    /* remove file with the specified name */
    files.erase(std::remove(files.begin(), files.end(), name), files.end());

    /* if there were files with this name, delete them */
    if (len != files.size()) {
        std::string path = folder + "/" + name;
        if (remove(path.data()) != 0) {
            std::cerr << "Error deleting file: No such file or directory\n";
        } else {
            std::cerr << "File " << name << " deleted.\n";
        }
    }
}

bool ServerNode::isUploadValid(const std::string &filename, uint64_t size) {
    /* upload is valid if there is enough memory, the filename is unique and doesn't contain a path */
    return this->memory >= size && std::find(files.begin(), files.end(), filename) == files.end() &&
           filename.find('/') == std::string::npos;
}

void ServerNode::closeConnections() {
    for (auto fd : fds) {
        if (fd.fd != -1) {
            if (filesUploaded.find(fd.fd) != filesUploaded.end()) {
                auto file = filesUploaded.at(fd.fd);
                fclose(file);
            }
            connection->closeSocket(fd.fd);
        }
    }
    std::exit(0);
}
