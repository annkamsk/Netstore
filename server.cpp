//
// Created by anna on 16.05.19.
//
#include "server.h"

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
