//
// Created by anna on 14.05.19.
//

#ifndef SIK2_SERVER_H
#define SIK2_SERVER_H

#include "group.h"

class ServerNode : Node {

private:
    long memory;

public:

    void greet() override;

    std::list<string> getList() override;

    std::list<string> getListWith(string pattern) override;

    char getFile(string name) override;

    void deleteFile(string name) override;

    void addFile(char *data) override;

};

void ServerNode::greet() {

}

std::list<string> ServerNode::getList() {
    return std::list<string>();
}

std::list<string> ServerNode::getListWith(string pattern) {
    return std::list<string>();
}

char ServerNode::getFile(string name) {
    return 0;
}

void ServerNode::deleteFile(string name) {

}

void ServerNode::addFile(char *data) {

}

#endif //SIK2_SERVER_H
