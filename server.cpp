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
        std::cerr << "Got TCP event.\n";
        this->handleTCPConnection(fds[1].fd);
    }
    for (size_t k = 2; k < fds.size(); ++k) {
        if (fds[k].fd != -1 && ((fds[k].revents & POLLIN) | POLLERR)) {
            fds[k].revents = 0;
            this->handleFileReceiving(fds[k].fd);
        } else if (fds[k].fd != -1) {
            /* check whether the socket is still open */
            int buff;
            if (recv(fds[k].fd, &buff, sizeof(buff), MSG_PEEK) == 0) {
                int sock = fds[k].fd;
                fclose(filesUploaded.at(sock));
                filesUploaded.erase(sock);
                close(sock);
                fds[k].fd = -1;
                // TODO clientRequests.erase(sock);
            }
        }
    }
    handleFileSending();
}

void ServerNode::handleFileReceiving(int sock) {
    if (filesUploaded.find(sock) != filesUploaded.end()) {
        auto file = filesUploaded.at(sock);
        if (this->connection->receiveFile(sock, file)) {
            std::cout << "File downloaded.\n";
            if (fclose(file) < 0) {
                throw FileException("Unable to close downloaded file.");
            }
            filesUploaded.erase(sock);
        }
    }
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
            // TODO close
            it = pendingFiles.erase(it);
        }
    }
}

void ServerNode::handleTCPConnection(int sock) {
    /* find a place for a new socket */
    size_t k = 2;
    for (; k < fds.size() && fds[k].fd != -1; ++k) {}
    if (k == fds.size()) {
        throw MessageSendException("Cannot accept the new connection now.");
    }

    /* accept the connection */
    sockaddr_in clientAddr{};
    socklen_t len = sizeof(clientAddr);
    int newSock;
    if ((newSock = accept(sock, (sockaddr *) &clientAddr, &len)) == -1) {
        throw MessageSendException("Error while accepting client's connection.");
    }
    /* save new file descriptor*/
    fds[k].fd = newSock;

    /* see what client requested before */
    std::string clientId = Netstore::getKey(clientAddr);
    if (clientRequests.find(clientId) == clientRequests.end()) {
        throw NetstoreException("Client " + std::string(inet_ntoa(clientAddr.sin_addr)) + " doesn't have any requests waiting.");
    }
    ClientRequest clientRequest = clientRequests.at(clientId);
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
            throw FileException("Unable to open file " + path);
        }
        filesUploaded.insert({newSock, f});
        this->connection->receiveFile(newSock, f);
    }
}

void ServerNode::handleUDPConnection(int sock) {
    auto request = this->connection->readFromUDPSocket(sock);
    auto message = MessageBuilder::build(request.getBuffer(), 0, request.getSize());

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
    response->completeMessage(memory,
                              std::vector<my_byte>(connection->getMcast().begin(), connection->getMcast().end()));
    connection->sendToSocket(sock, request.getCliaddr(), response);
}


void ServerNode::prepareDownload(int sock, const ConnectionResponse &request, const std::shared_ptr<Message> &message) {
    std::string filename(message->getData().begin(), message->getData().end());

    if (std::find(files.begin(), files.end(), filename) != files.end()) {
        auto port = (uint64_t) this->connection->getTCPport();
        /* save request */
        auto clientAddr = request.getCliaddr();
        clientRequests.insert({Netstore::getKey(clientAddr), {clock(), filename, true, false}});

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
        clientRequests.insert({Netstore::getKey(clientAddr), {clock(), filename, false, false}});

        auto response = MessageBuilder::create("CAN_ADD");
        response->setSeq(message->getCmdSeq());
        response->completeMessage(port, std::vector<my_byte>(filename.begin(), filename.end()));
        connection->sendToSocket(sock, request.getCliaddr(), response);
    } else {
        auto response = MessageBuilder::create("NO_WAY");
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
            connection->closeSocket(fd.fd);
        }
    }
}
