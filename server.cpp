#include "server.h"
#include "group.h"

void ServerNode::addFile(const std::string &filename, unsigned long long size) {
    Node::addFile(filename, 0);
    memory -= size;
    std::cout << filename << std::endl;
}

std::shared_ptr<Connection> ServerNode::startConnection() {
    auto connection = Node::startConnection();
    connection->openSocket();
    connection->addToMcast();
    connection->addToLocal(this->getConnection()->getPort());
    return connection;
}


