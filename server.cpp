#include "server.h"

void ServerNode::addFile(const string& filename, unsigned long long size) {
    files.push_back(filename);
    memory -= size;
    std::cout << filename <<std::endl;
}


