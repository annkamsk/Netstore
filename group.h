#include <utility>

#include <utility>
#include <boost/algorithm/string.hpp>
#include <thread>


#ifndef SIK2_GROUP_H
#define SIK2_GROUP_H

#include "Connection.h"

class Node {
private:
    std::vector<std::string> files;
    std::string folder;
    std::shared_ptr<UDPConnection> connection;
    std::vector<TCPConnection> tcps;
    std::vector<std::thread> ths;

public:
    Node(std::string mcast, unsigned port, unsigned timeout, std::string folder) :
            folder(std::move(folder)),
            connection(std::make_shared<UDPConnection>(UDPConnection(std::move(mcast), port, timeout))) {}

    virtual std::shared_ptr<Connection> startConnection() {
        return this->connection;
    }
    std::shared_ptr<UDPConnection> getConnection() {
        return this->connection;
    }

    const std::vector<TCPConnection> &getTcps() const {
        return tcps;
    }

    void addTcp(const TCPConnection &tcp) {
        this->tcps.emplace_back(tcp);
    }

    const std::vector<std::thread> &getThs() {
        return ths;
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
        (void)size;
    }

    virtual void addFile(const std::string &filename, sockaddr_in addr) {
        this->files.push_back(filename);
        (void)addr;
    }

};


#endif //SIK2_GROUP_H
