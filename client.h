//
// Created by anna on 14.05.19.
//

#ifndef SIK2_CLIENT_H
#define SIK2_CLIENT_H

#include "group.h"


class ClientNode : Node {
public:
    void greet() override;

    std::list<string> getList() override;

    std::list<string> getListWith(string pattern) override;

    char getFile(string name) override;

    void deleteFile(string name) override;

    void addFile(char *data) override;
};

void ClientNode::greet() {

}

std::list<string> ClientNode::getList() {
    return std::list<string>();
}

std::list<string> ClientNode::getListWith(string pattern) {
    return std::list<string>();
}

char ClientNode::getFile(string name) {
    return 0;
}

void ClientNode::deleteFile(string name) {

}

void ClientNode::addFile(char *data) {

}

#endif //SIK2_CLIENT_H
