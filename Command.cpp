#include "Command.h"

std::shared_ptr<Command> CommandBuilder::build(const std::vector<char>& data) {
    if (data.size() < Netstore::MIN_SMPL_CMD_SIZE) {
        throw InvalidMessageException();
    }

    /* set cmd */
    std::string cmd = this->parseCmd(data);
    auto command = commandStrings.at(cmd);

    /* set cmd_seq */
    command->setSeq(parseNum(data, 10, 18));

    uint ite = 18;
    if (command->isComplex()) {
        if (data.size() < Netstore::MIN_CMPLX_CMD_SIZE) {
            throw InvalidMessageException();
        }
        /* set param */
        command->setParam(parseNum(data, 18, 26));
        ite += 8;
    }

    /* set data */
    command->setData(parseData(data, ite));
    return command;
}

std::string CommandBuilder::parseCmd(const std::vector<char>& data) {
    /* get alpha chars from first 10 chars of data */
    std::string cmd{};
    for (auto ite = data.begin(); ite < data.begin() + 10; ++ite) {
        if (!isalpha(*ite)) break;
        cmd.push_back(*ite);
    }

    /* if the command is not correct, throw exception */
    if (commandStrings.find(cmd) == commandStrings.end()) {
        throw InvalidMessageException();
    }
}

uint64_t CommandBuilder::parseNum(const std::vector<char> &data, uint32_t from, uint32_t to) {
    /* check if number was given */
    if (!std::all_of(data.begin() + from, data.begin() + to,
                     [](char c) { return isdigit(c); })) {
        throw InvalidMessageException();
    }
    std::string seq = std::vector(data.begin() + from, data.begin() + to).data();
    return std::stoi(seq);
}

std::shared_ptr<char> CommandBuilder::parseData(const std::vector<char>& data, uint from) {

}
