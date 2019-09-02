#include "client.h"

void ClientNode::listen() {
    poll(&fds[0], N, 0);

    /* handle event on a STDIN */
    if (fds[0].revents & POLLIN) {
        fds[0].revents = 0;
        readUserInput();
        std::cout << "Write a command: \n";
    }

    for (size_t k = 1; k < fds.size(); ++k) {
        if (fds[k].fd != -1 && ((fds[k].revents & POLLIN) | POLLERR)) {
            fds[k].revents = 0;
            handleFileReceiving(fds[k].fd);
        } else if (fds[k].fd != -1) {
            /* check whether the socket is still open */
            int buff;
            if (recv(fds[k].fd, &buff, sizeof(buff), MSG_PEEK) == 0) {
                handleFileDownloaded(fds[k].fd);
                fds[k].fd = -1;
            }
        }
    }
    handleFileSending();
}

void ClientNode::handleFileReceiving(int fd) {
    if (this->downloadRequests.find(fd) != downloadRequests.end()) {
        auto f = this->downloadRequests.at(fd).f;
        if (this->connection->receiveFile(fd, f)) {
            handleFileDownloaded(fd);
        }
    }
}

void ClientNode::handleFileDownloaded(int fd) {
    auto request = downloadRequests.at(fd);
    std::cout << "File " << request.filename << " downloaded" << " (" << inet_ntoa(request.server.sin_addr) << ":"
              << ntohl(request.server.sin_port) << ")\n";
    if (fclose(request.f) < 0) {
        throw FileException("Unable to close downloaded file.");
    }
    downloadRequests.erase(fd);
}

void ClientNode::handleFileSending() {
    auto it = uploadFiles.begin();
    while (it != uploadFiles.end()) {
        auto f = *it;
        try {
            if (f.handleSending()) {
                std::cout << "File " + f.getFilename() + " uploaded (" + inet_ntoa(f.getClientAddr().sin_addr) + ":" +
                             std::to_string(ntohl(f.getClientAddr().sin_port)) + ")\n";
                it = uploadFiles.erase(it);
            } else {
                ++it;
            }
        } catch (NetstoreException &e) {
            std::cout << "File " + f.getFilename() + " uploading failed (" + inet_ntoa(f.getClientAddr().sin_addr) + ":" +
                       std::to_string(ntohl(f.getClientAddr().sin_port)) + ")" + e.what() + " " + e.details() + "\n";
            // TODO close
            it = uploadFiles.erase(it);
        }
    }
}

void ClientNode::readUserInput() {
    std::string input;
    std::getline(std::cin, input);

    size_t n = input.find(' ');
    std::string command = n == std::string::npos ? input : input.substr(0, n);
    std::string args = n == std::string::npos ? "" : input.substr(n + 1, input.size());

    /* client should be case insensitive, so let's transform string to lower case */
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    /* if input is not of form: 'command( \w+)\n' */
    if (this->commands.find(command) == this->commands.end()) {
        throw InvalidInputException();
    }

    /* execute function associated with given command */
    this->commands.at(command)(args);
}

void ClientNode::startConnection() {
    if ((this->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        syserr("socket");
    }
    /* bind */
    struct sockaddr_in local_address{};
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(0);
    if (bind(this->sock, (struct sockaddr *) &local_address, sizeof local_address) < 0) {
        syserr("bind");
    }
    connection->setReceiver();
    connection->setTimeout(this->sock);

    /* listen on user's input */
    fds[0] = {fileno(stdin), POLLIN, 0};
}


void ClientNode::discover() {
    auto message = messageBuilder.create("HELLO");
    this->connection->multicast(this->sock, message);

    // TODO wait for TTL for responses
    auto response = this->connection->readFromUDPSocket(this->sock);

    try {
        auto responseMessage = messageBuilder.build(response.getBuffer(), message->getCmdSeq(), response.getSize());

        /* save the server with its free space */
        auto address = response.getCliaddr();
        auto space = responseMessage->getParam();
        if (this->memory.find(space) == this->memory.end()) {
            this->memory.insert({space, std::queue<sockaddr_in>()});
        }
        this->memory.at(space).push(address);
        std::cout << "Found " << inet_ntoa(address.sin_addr) << " (" << responseMessage->getData().data()
                  << ") with free space " << space << "\n";
    } catch (std::exception &e) {
        std::cerr << "[PCKG ERROR]  Skipping invalid package from " << inet_ntoa(response.getCliaddr().sin_addr) << ":"
                  << response.getCliaddr().sin_port << ". Reason: " << e.what() << "\n";
    }
}

void ClientNode::search(const std::string &s) {
    auto message = messageBuilder.create("LIST");
    message->setData(std::vector<my_byte>(s.begin(), s.end()));
    this->connection->multicast(this->sock, message);

    // TODO wait TTL for response
    auto response = this->connection->readFromUDPSocket(this->sock);
    try {
        auto responseMessage = messageBuilder.build(response.getBuffer(), message->getCmdSeq(), response.getSize());

        /* get filenames from data */
        std::vector<std::string> filenames;
        boost::split(filenames, responseMessage->getData(), [](char c) { return c == '\n'; });

        /* delete filenames from previous search */
        this->files.clear();
        /* save filenames and print them out */
        for (const auto &f : filenames) {
            std::cout << f << " (" << inet_ntoa(response.getCliaddr().sin_addr) << ")\n";
            addFile(f, response.getCliaddr());
        }
    } catch (WrongSeqException &e) {
        std::cerr << "[PCKG ERROR]  Skipping invalid package from " << response.getCliaddr().sin_addr.s_addr << ":"
                  << response.getCliaddr().sin_port << ".\n";
    }
}

void ClientNode::fetch(const std::string &s) {
    auto servers = this->files.find(s);
    if (servers == this->files.end() || servers->second.empty()) {
        throw FileException("File not found on any server.");
    }
    auto message = messageBuilder.create("GET");
    message->setData(std::vector<my_byte>(s.begin(), s.end()));

    /* choose provider and move it to the end of queue */
    auto provider = servers->second.front();
    servers->second.pop();
    servers->second.push(provider);

    try {
        /* send request for file */
        this->connection->sendToSocket(sock, provider, message);

        /* read a response */
        auto response = this->connection->readFromUDPSocket(this->sock);
        auto responseMessage = messageBuilder.build(response.getBuffer(), message->getCmdSeq(), response.getSize());
        std::string filename = std::string(responseMessage->getData().begin(), responseMessage->getData().end());
        /* if the filename is wrong */
        if (filename != s) {
            throw NetstoreException("Server wants to send a wrong file.");
        }

        int tcp = connectWithServer(responseMessage->getParam(), provider);

        FILE *f;
        std::string path(folder + "/" + filename);
        if ((f = fopen(path.data(), "wb")) == nullptr) {
            throw FileException("Could not open file " + path + " for writing.");
        }
        downloadRequests.insert({tcp, {path, provider, f}});

    } catch (NetstoreException &e) {
        std::cout << "File " << s << " downloading failed (" << inet_ntoa(provider.sin_addr) << ":" << provider.sin_port
                  << ")" << " Reason: " << e.what() << " " << e.details() << "\n";
        // TODO close sock
    }
}

void ClientNode::upload(const std::string &s) {
    /* find file */
    fs::path file(s);
    if (!fs::exists(s) || !fs::is_regular_file(file)) {
        std::cout << "ERROR: File " + s + " does not exist\n";
        return;
    }

    /* get server with max memory space */
    auto max = this->memory.rbegin();

    /* check if there is enough space */
    uint64_t size = fs::file_size(file);
    if (size > max->first) {
        throw FileException("File " + s + " too big.");
    }

    auto server = max->second.front();
    max->second.pop();
    max->second.push(server);

    auto message = messageBuilder.create("ADD");
    message->completeMessage(htobe64(size), std::vector<my_byte>(s.begin(), s.end()));

    try {
        /* send request for upload */
        this->connection->sendToSocket(this->sock, server, message);

        /* read a response */
        auto response = this->connection->readFromUDPSocket(this->sock);
        auto responseMessage = messageBuilder.build(response.getBuffer(), message->getCmdSeq(), response.getSize());
        if (responseMessage->getCmd() == "NO_WAY") {
            throw NetstoreException("Server does not agree for uploading this file.");
        }

        std::string filename = std::string(responseMessage->getData().begin(), responseMessage->getData().end());
        /* if the filename is wrong */
        if (filename != s) {
            throw NetstoreException("Server wants to receive a wrong file.");
        }

        int tcp = connectWithServer(responseMessage->getParam(), server);
        FileSender sender{};
        sender.init(filename, tcp, server);
        uploadFiles.push_back(sender);

    } catch (NetstoreException &e) {
        std::cout << "File " + s + " uploading failed (" + inet_ntoa(server.sin_addr) + ":" +
                     std::to_string(ntohl(server.sin_port)) + ") " + e.what() + e.details() + "\n";
        // TODO close
    }

}

int ClientNode::connectWithServer(uint64_t param, sockaddr_in &addr) {
    auto port = (uint16_t) param;
    addr.sin_port = htons(port);
    int tcp = connection->openTCPSocket(addr);

    size_t i = 0;
    for (; i < fds.size() && fds.at(i).fd != -1; ++i) {}
    if (i == fds.size()) {
        throw NetstoreException("Too many open connections. Try again later.");
    }
    fds.at(i) = {tcp, POLLIN, 0};
    return tcp;
}

void ClientNode::remove(const std::string &s) {
    if (s.empty()) {
        throw InvalidInputException();
    }
    auto message = messageBuilder.create("DEL");
    message->setData(std::vector<my_byte>(s.begin(), s.end()));
    this->connection->multicast(this->sock, message);
}

void ClientNode::exit() {
    for (auto fd : fds) {
        if (fd.fd != -1) {
            connection->closeSocket(fd.fd);
            downloadRequests.erase(fd.fd);
            fd.fd = -1;
        }
    }
}

void ClientNode::addFile(const std::string &filename, sockaddr_in addr) {
    this->files[filename].push(addr);
}

