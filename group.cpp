
#include "group.h"
#include "Command.h"

std::shared_ptr<Connection> Node::startConnection() {
    std::shared_ptr<Connection> connection = std::make_shared<UDPConnection>(this->group.getCMD_PORT());
    connection->openSocket();
    connection->addToMcast(this->group.getMCAST_ADDR());
    connection->addToLocal();
    return connection;
}

