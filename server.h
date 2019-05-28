
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

    unsigned long long getMemory() override {
        return memory;
    }

    void setMemory(unsigned long long int memory) {
        ServerNode::memory = memory;
    }

    unsigned int getTimeout() const {
        return timeout;
    }

    void setTimeout(unsigned int timeout) {
        ServerNode::timeout = timeout;
    }

    void setFolder(const string &folder) {
        ServerNode::folder = folder;
    }

    const vector<string> &getFiles() const override {
        return files;
    }

    void setFiles(const vector<string> &files) {
        ServerNode::files = files;
    }

    void addFile(const string& filename, unsigned long long size);

    std::shared_ptr<Connection> startConnection() override;
};

#endif //SIK2_SERVER_H
