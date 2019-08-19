#include <fstream>
#include "client.h"

void ClientNode::readUserInput() {
    std::string input;
    std::cin >> input;

    size_t n = input.find(' ');
    std::string command = n == std::string::npos ? input : input.substr(0, n);
    std::string args = input.substr(n + 1, input.size());

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
    this->sock = connection->openUDPSocket();
    connection->setReceiver();
}

void ClientNode::discover() {
    SimpleMessage message("HELLO");
    this->connection->multicast(this->sock, message.getRawData());

    // TODO wait for TTL for responses
    auto response = this->connection->readFromUDPSocket(this->sock);
    try {
        auto responseMessage = MessageBuilder::build(response.getBuffer(), 0);
        std::cout << "Found " << response.getCliaddr().sin_addr.s_addr << " (" << responseMessage->getData().data()
                  << ") with free space " << responseMessage->getParam() << "\n";
//    Dla każdego odnalezionego serwera klient powinien wypisać na standardowe wyjście w jednej linii adres jednostkowy IP tego serwera,
//    następnie w nawiasie adres MCAST_ADDR otrzymany od danego serwera, a na końcu rozmiar dostępnej przestrzeni dyskowej na tym serwerze.
//    Found 10.1.1.28 (239.10.11.12) with free space 23456

    } catch (WrongSeqException &e) {
        std::cerr << "[PCKG ERROR]  Skipping invalid package from " << response.getCliaddr().sin_addr.s_addr << ":"
                  << response.getCliaddr().sin_port << ".\n";
//        Autor programu powinien uzupełnić wiadomość po kropce o dodatkowe informacje opisujące błąd, ale bez użycia znaku nowej linii.
    }
}


void ClientNode::search(const std::string &s) {
    auto message = SimpleMessage("LIST");
    message.setData(std::vector<char>(s.begin(), s.end()));
    this->connection->multicast(this->sock, message.getRawData());

    // TODO wait TTL for response
    auto response = this->connection->readFromUDPSocket(this->sock);
    try {
        auto responseMessage = MessageBuilder::build(response.getBuffer(), message.getCmdSeq());

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
        std::cout << "File " << s << " was not found on any server.\n";
        return;
    }
    auto message = SimpleMessage("GET");
    message.setData(std::vector<char>(s.begin(), s.end()));

    /* choose provider and move it to the end of queue */
    auto provider = servers->second.front();
    servers->second.pop();
    servers->second.push(provider);

    try {
        /* send request for file */
        this->getConnection()->sendToSocket(provider, message.getRawData());
    } catch (NetstoreException &e) {
        std::cout << "File " << s << " downloading failed (" << inet_ntoa(provider.sin_addr) << ":" << provider.sin_port
                  << ")" << " Reason: " << e.what();
    }
}

void ClientNode::fetchFile(ComplexMessage message) {

}

void ClientNode::upload(const std::string &s) {
    /* find file */
    fs::path file(s);
    if (!fs::exists(s) || !fs::is_regular_file(file)) {
        std::cout << "File " + s + " does not exist\n";
        return;
    }

    /* get servers with max memory space */
    auto max = space.rbegin();

    /* check if there is enough place */
    uint64_t size = fs::file_size(file);
    if (size > max->first) {
        std::cout << "File " << s << " too big\n";
        return;
    }

    auto dest = max->second.front();
    max->second.pop();

    try {

        // upload - not blocking
        std::cout << "File " << s << " uploaded (" << inet_ntoa(dest.sin_addr) << ":" << dest.sin_port << ")\n";

        /* index new file and server with new spare space */
        this->files[s].push(dest);
        uint64_t spaceLeft = max->first - size;
        this->space[spaceLeft].push(dest);
    } catch (std::exception &e) {
        std::cout << "File " << s << " uploading failed (" << inet_ntoa(dest.sin_addr) << ":" << dest.sin_port
                  << ") Reason: " << e.what();
    }
}

void ClientNode::remove(const std::string &s) {
    if (s.empty()) {
        throw InvalidInputException();
    }
    auto message = SimpleMessage("DELETE");
    message.setData(std::vector(s.begin(), s.end()));
    this->connection->multicast(message.getRawData());
    std::cout << "Request to remove file " + s + " sent.\n";
}

void ClientNode::exit() {
//    klient po otrzymaniu tego polecenia powinien zakończyć wszystkie otwarte
//    połączenia i zwolnić pobrane zasoby z systemu oraz zakończyć pracę aplikacji.
}

void ClientNode::addFile(const std::string &filename, sockaddr_in addr) {
    this->files[filename].push(addr);
}

