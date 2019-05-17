#include <utility>

#include <utility>

#include <utility>

//
// Created by anna on 14.05.19.
//

#ifndef SIK2_SERVER_H
#define SIK2_SERVER_H

#include "group.h"

class ServerNode : public Node {

private:
    unsigned long long memory{};
    unsigned int timeout{};
    string folder;
    vector<string> files;

public:
    ServerNode(Group group, unsigned long long memory, unsigned int timeout, string folder) :
        Node(std::move(group)), memory(memory), timeout(timeout), folder(std::move(folder)), files(vector<string>()) {};

    string getFolder() {
        return folder;
    }

    void addFile(const string& filename, unsigned long long size);

    void listen();

    void getList(Command command);

    void getListWith(Command command);

    char getFile(Command command);

    void deleteFile(Command command);

    void addFile(Command command);
};

#endif //SIK2_SERVER_H
