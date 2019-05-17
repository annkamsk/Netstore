#include <iostream>
#include "server.h"

void ServerNode::greet() {

}

void ServerNode::getList(Command command) {

}

void ServerNode::getListWith(Command command) {
}

char ServerNode::getFile(Command command) {
    return 0;
}

void ServerNode::deleteFile(Command command) {

}

void ServerNode::addFile(Command command) {

}

void ServerNode::addFile(const string& filename, unsigned long long size) {
    files.push_back(filename);
    memory -= size;
    std::cout << filename <<std::endl;
}

void ServerNode::listen() {
    for (;;) {
        vector<char> buffer = this->connection.readFromSocket(Netstore::MIN_SIZE);
        Command command(buffer);
        if (command.isAddFile()) {
            
        }

    }
}
