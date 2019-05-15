#include <utility>

#include <utility>

//
// Created by anna on 14.05.19.
//

#ifndef SIK2_SERVER_H
#define SIK2_SERVER_H

#include "group.h"

class ServerNode : Node {

private:
    unsigned long long memory;
    unsigned int timeout;
    string folder;
    vector<string> files;

public:
    ServerNode(Group group, unsigned long long memory, string folder, unsigned int timeout) :
        Node(std::move(group)), memory(memory), folder(std::move(folder)), timeout(timeout), files(vector<string>()) {};

    string getFolder() {
        return folder;
    }

    void greet() override;

    std::list<string> getList() override;

    std::list<string> getListWith(string pattern) override;

    char getFile(string name) override;

    void deleteFile(string name) override;

    void addFile(char *data) override;

    void addFile(const string& filename, unsigned long long size);

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

void ServerNode::addFile(const string& filename, unsigned long long size) {
    files.push_back(filename);
    memory -= size;
}

#endif //SIK2_SERVER_H
