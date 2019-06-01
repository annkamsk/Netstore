#include <fstream>
#include "client.h"

std::shared_ptr<Connection> ClientNode::startConnection() {
    std::shared_ptr<Connection> connection = Node::getConnection();
    connection->openSocket();
    connection->activateBroadcast();
    connection->addToLocal();
    connection->setReceiver();
    return connection;
}

void ClientNode::readUserInput() {
    std::string command{};
    std::getline(std::cin, command);

    std::vector<std::string> tokens;
    boost::split(tokens, command, [](char c) { return c == ' '; });

    if (tokens.size() > 2 || tokens.empty()) {
        throw InvalidInputException();
    }

    /* client should be case insensitive, so let's transform string to lower case */
    std::transform(tokens.at(0).begin(), tokens.at(0).end(), tokens.at(0).begin(), ::tolower);

    /* if input is not of form: 'command %s\n' */
    if (this->commands.find(tokens.at(0)) == this->commands.end()) {
        throw InvalidInputException();
    }
    /* return message associated with given input */
    commands.at(tokens.at(0))(tokens.size() > 1 ? tokens.at(1) : "");
}

void ClientNode::discover() {
    auto message = SimpleGreetMessage();
    Node::getConnection()->broadcast(message.getRawData());
    // block ui
    // wait for TTL for responses
    auto response = Node::getConnection()->readFromSocket();
    auto responseMessage = MessageBuilder().build(response.getBuffer());
    std::cout << "Found " << response.getCliaddr().sin_addr.s_addr << " (" << responseMessage->getData().data()
              << ") with free space " << responseMessage->getParam() << "\n";
//    Dla każdego odnalezionego serwera klient powinien wypisać na standardowe wyjście w jednej linii adres jednostkowy IP tego serwera,
//    następnie w nawiasie adres MCAST_ADDR otrzymany od danego serwera, a na końcu rozmiar dostępnej przestrzeni dyskowej na tym serwerze.
//    Found 10.1.1.28 (239.10.11.12) with free space 23456

}

void ClientNode::search(const std::string &s) {
    auto message = SimpleListMessage();
    message.setData(std::vector<char>(s.begin(), s.end()));
    Node::getConnection()->broadcast(message.getRawData());
//    {nazwa_pliku} ({ip_serwera})
//  add file
}

void ClientNode::fetch(const std::string &s) {
    if (std::find(this->getFiles().begin(), this->getFiles().end(), s) == this->getFiles().end()) {
        throw InvalidInputException();
    }
    auto message = SimpleGetMessage();
    message.setData(std::vector<char>(s.begin(), s.end()));
    Node::getConnection()->broadcast(message.getRawData());
    // download file
}

void ClientNode::upload(const std::string &s) {
    if (FILE *file = fopen(s.c_str(), "r")) {
        fclose(file);
    } else {
        std::cout << "File " + s + "does not exist\n";
    }
}

void ClientNode::remove(const std::string &s) {
    if (s.empty()) {
        throw InvalidInputException();
    }
    auto message = SimpleDeleteMessage();
    message.setData(std::vector(s.begin(), s.end()));
    this->getConnection()->broadcast(message.getRawData());
//    klient po otrzymaniu tego polecenia powinien wysłać do grupy serwerów zlecenie usunięcia wskazanego przez
//    użytkownika pliku.
}

void ClientNode::exit() {
//    klient po otrzymaniu tego polecenia powinien zakończyć wszystkie otwarte
//    połączenia i zwolnić pobrane zasoby z systemu oraz zakończyć pracę aplikacji.
}

