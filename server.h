
#ifndef SIK2_SERVER_H

#define SIK2_SERVER_H
#include "group.h"

#include <utility>

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

    unsigned long long getMemory() {
        return memory;
    }

    void addFile(const string& filename, unsigned long long size);


};

#endif //SIK2_SERVER_H
