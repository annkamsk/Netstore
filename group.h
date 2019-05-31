#include <utility>
#include <boost/algorithm/string.hpp>


#ifndef SIK2_GROUP_H
#define SIK2_GROUP_H

#include "Connection.h"

class Node {
private:
    std::vector<std::string> files;
    std::string folder;
    std::shared_ptr<UDPConnection> connection;
public:
    Node(std::string mcast, unsigned port, unsigned timeout, std::string folder) :
            folder(std::move(folder)),
            connection(std::make_shared<UDPConnection>(UDPConnection(mcast, port, timeout))) {}

    virtual std::shared_ptr<Connection> startConnection() {
        return this->connection;
    }

    std::shared_ptr<UDPConnection> getConnection() {
        return this->connection;
    }

    virtual unsigned long long getMemory() {
        return 0;
    }

    std::string getFolder() {
        return folder;
    }

    const std::vector<std::string> &getFiles() const {
        return files;
    }

    virtual void addFile(const std::string &filename, unsigned long long size) {
        this->files.push_back(filename);
    }

};


#endif //SIK2_GROUP_H
