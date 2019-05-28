#include "server.h"

void ServerNode::addFile(const string &filename, unsigned long long size) {
    files.push_back(filename);
    memory -= size;
    std::cout << filename << std::endl;
}

std::shared_ptr<Connection> ServerNode::startConnection() {
    std::shared_ptr<Connection> connection = std::make_shared<UDPConnection>(this->group.getMCAST_ADDR(),
                                                                             this->group.getCMD_PORT(),
                                                                             this->getTimeout());
    connection->openSocket();
    connection->addToMcast();
    connection->addToLocal();
    return connection;
}


