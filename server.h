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
    unsigned long long memory{};
    unsigned int timeout{};
    string folder;
    vector<string> files;

public:
    ServerNode() = default;
    ServerNode(Group group, unsigned long long memory, unsigned int timeout, string folder) :
        Node(std::move(group)), memory(memory), timeout(timeout), folder(std::move(folder)), files(vector<string>()) {};

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

    void addToMcast();

};

#endif //SIK2_SERVER_H
