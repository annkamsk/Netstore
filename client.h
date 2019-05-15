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

#endif //SIK2_CLIENT_H
