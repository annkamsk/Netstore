#ifndef SIK2_CLIENT_H
#define SIK2_CLIENT_H

#include "group.h"
#include "Message.h"
#include <wait.h>
#include <boost/algorithm/string.hpp>

class ClientNode : public Node {
private:
    void discover();

    void search(const std::string &s);

    void fetch(const std::string &s);

    void upload(const std::string &s);

    void remove(const std::string &s);

    void exit();

    std::map<std::string, std::function<void(std::string)>> commands = {
            {"discover", [=](const std::string &s) { this->discover(); }},
            {"search",   [=](const std::string &s) { this->search(s); }},
            {"fetch",    [=](const std::string &s) { this->fetch(s); }},
            {"upload",   [=](const std::string &s) { this->upload(s); }},
            {"remove",   [=](const std::string &s) { this->remove(s); }},
            {"exit",     [=](const std::string &s) { this->exit(); }}
    };
public:

    ClientNode(const std::string &mcast, unsigned port, unsigned int timeout, const std::string &folder) :
            Node(mcast, port, timeout, folder) {}

    std::shared_ptr<Connection> startConnection() override;

    void readUserInput();

};

#endif //SIK2_CLIENT_H
