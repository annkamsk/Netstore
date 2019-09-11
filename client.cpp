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
              << ntohs(request.server.sin_port) << ")\n";
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
                std::cout << "File " + f.getFilename() + " uploaded (" + inet_ntoa(f.getClientAddr().sin_addr) + ":" <<
                             ntohs(f.getClientAddr().sin_port) << ")\n";
                it = uploadFiles.erase(it);
            } else {
                ++it;
            }
        } catch (NetstoreException &e) {
            std::cout
                    << "File " + f.getFilename() + " uploading failed (" + inet_ntoa(f.getClientAddr().sin_addr) + ":"<<
                    ntohs(f.getClientAddr().sin_port) << ")" << e.what() << " " << e.details() << "\n";
            it->closeConnection();
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

    /* delete servers from previous search */
    this->memory.clear();
    try {
        time_t start = time(nullptr);
        time_t now = time(nullptr);
        pollfd udp = {this->sock, POLLIN, 0};
        /* take only responses from ttl period */
        while (now - start < this->connection->getTtl()) {
            if (poll(&udp, 1, 0) > 0) {
                auto response = this->connection->readFromUDPSocket(this->sock);
                auto responseMessage = messageBuilder.build(response.getBuffer(), message->getCmdSeq(), response.getSize());

                /* save the server with its free space */
                auto address = response.getCliaddr();
                auto space = responseMessage->getParam();
                if (this->memory.find(space) == this->memory.end()) {
                    this->memory.insert({space, std::vector<sockaddr_in>()});
                }
                this->memory.at(space).push_back(address);
                std::cout << "Found " << inet_ntoa(address.sin_addr) << " (" << responseMessage->getData().data()
                          << ") with free space " << space << "\n";
            }
            now = time(nullptr);
        }
    } catch (NetstoreException &e) {
        std::cerr << e.what() << " " << e.details();
    }
}

void ClientNode::search(const std::string &s) {
    auto message = messageBuilder.create("LIST");
    message->setData(std::vector<my_byte>(s.begin(), s.end()));
    this->connection->multicast(this->sock, message);

    /* delete filenames from previous search */
    this->files.clear();
    try {
        time_t start = time(nullptr);
        time_t now = time(nullptr);
        pollfd udp = {this->sock, POLLIN, 0};

        /* take only responses from ttl period */
        while (now - start < this->connection->getTtl()) {
            if (poll(&udp, 1, 0) > 0) {
                udp.revents = 0;
                auto response = this->connection->readFromUDPSocket(this->sock);
                auto responseMessage = messageBuilder.build(response.getBuffer(), message->getCmdSeq(),
                                                            response.getSize());

                /* get filenames from data */
                std::vector<std::string> filenames;
                boost::split(filenames, responseMessage->getData(), [](char c) { return c == '\n'; });

                /* save filenames and print them out */
                for (const auto &f : filenames) {
                    if (!std::all_of(f.begin(), f.end(), isspace)) {
                        std::cout << f << " (" << inet_ntoa(response.getCliaddr().sin_addr) << ")\n";
                        addFile(f, response.getCliaddr());
                    }
                }
            }
            now = time(nullptr);
        }
    } catch (WrongSeqException &e) {
        std::cerr << e.what() << e.details();
    }
}

void ClientNode::fetch(const std::string &s) {
    auto servers = this->files.find(s);
    if (servers == this->files.end() || servers->second.empty()) {
        throw FileException("File not found on any server.");
    }
    auto message = messageBuilder.create("GET");
    message->setData(std::vector<my_byte>(s.begin(), s.end()));

    /* choose provider */
    auto provider = servers->second.front();

    int newPort = 0;
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
        newPort = responseMessage->getParam();
        int tcp = connectWithServer(newPort, provider);

        FILE *f;
        std::string path(folder + "/" + filename);
        if ((f = fopen(path.data(), "wb")) == nullptr) {
            throw FileException("Could not open file " + path + " for writing.");
        }
        downloadRequests.insert({tcp, {path, provider, f}});

    } catch (NetstoreException &e) {
        std::cout << "File " << s << " downloading failed (" << inet_ntoa(provider.sin_addr) << ":" << newPort
                  << ")" << " Reason: " << e.what() << " " << e.details() << "\n";
    }
}

void ClientNode::upload(const std::string &s) {
    /* find file */
    fs::path file(s);
    if (!fs::exists(s) || !fs::is_regular_file(file)) {
        std::cout << "ERROR: File " + s + " does not exist\n";
        return;
    }
    size_t size = fs::file_size(file);

    /* get server with max memory space */
    auto server = findServer(s, size);

    auto message = messageBuilder.create("ADD");
    message->completeMessage(htobe64(size), std::vector<my_byte>(s.begin(), s.end()));

    int newPort = 0;
    try {
        /* send request for upload */
        this->connection->sendToSocket(this->sock, server, message);

        /* read a response */
        auto response = this->connection->readFromUDPSocket(this->sock);
        auto responseMessage = messageBuilder.build(response.getBuffer(), message->getCmdSeq(), response.getSize());
        if (responseMessage->getCmd() == "NO_WAY") {
            throw NetstoreException("Server does not agree for uploading this file.");
        }

        std::string filename(responseMessage->getData().begin(), responseMessage->getData().end());
        /* if the filename is wrong */
        if (filename != s) {
            throw NetstoreException("Server wants to receive a wrong file.");
        }
        newPort = responseMessage->getParam();
        int tcp = connectWithServer(newPort, server);
        FileSender sender{};
        sender.init(filename, tcp, server);
        uploadFiles.push_back(sender);

    } catch (NetstoreException &e) {
        std::cout << "File " + s + " uploading failed (" + inet_ntoa(server.sin_addr) + ":" << newPort << ") "
                  << e.what() << " " << e.details() << "\n";
    }

}

int ClientNode::connectWithServer(uint64_t param, sockaddr_in &addr) {
    auto port = (uint16_t) param;
    addr.sin_port = htons(port);
    int tcp = connection->openTCPSocket(addr);

    size_t i = 0;
    for (; i < fds.size() && fds.at(i).fd != -1; ++i) {}
    if (i == fds.size()) {
        close(tcp);
        throw NetstoreException("Too many open connections. Try again later.");
    }
    fds.at(i) = {tcp, POLLIN, 0};
    return tcp;
}

sockaddr_in ClientNode::findServer(const std::string &s, size_t size) {
    auto excluded = this->files.find(s);
    std::vector<sockaddr_in> serversExcluded = excluded == files.end() ? std::vector<sockaddr_in>{} : this->files.at(s);

    /* iterate servers starting from these with the biggest memory */
    for (auto &it : memory) {
        if (size > it.first) {
            throw FileException("File " + s + " too big.");
        }
        for (auto &server : it.second) {
            if (!hasFile(server, s)) {
                /* if it doesn't have the file yet */
                return server;
            }
        }
    }
    throw FileException("File " + s + " too big.");
}

bool ClientNode::hasFile(sockaddr_in server, const std::string& file) {
    if (this->files.find(file) == this->files.end()) {
        return false;
    }
    std::vector<sockaddr_in> excluded = this->files.at(file);
    return std::find_if(excluded.begin(), excluded.end(),
                        [&server](sockaddr_in addr) { return addr.sin_addr.s_addr == server.sin_addr.s_addr; }) !=
           excluded.end();
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
        if (fd.fd != -1 && fd.fd != 0) {
            if (downloadRequests.find(fd.fd) != downloadRequests.end()) {
                auto request = downloadRequests.at(fd.fd);
                fclose(request.f);
            }
            downloadRequests.erase(fd.fd);
            connection->closeSocket(fd.fd);
        }
    }
    std::exit(0);
}

void ClientNode::addFile(const std::string &filename, sockaddr_in addr) {
    this->files[filename].push_back(addr);
}

