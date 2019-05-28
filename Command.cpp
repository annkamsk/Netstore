#include "Command.h"
#include "server.h"

std::shared_ptr<Command> CommandBuilder::build(const std::vector<char> &data) {
    if (data.size() < Netstore::MIN_SMPL_CMD_SIZE) {
        throw InvalidMessageException();
    }

    /* set cmd */
    std::string cmd = this->parseCmd(data);
    auto command = commandStrings.at(cmd);

    /* set cmd_seq */
    command->setSeq(parseNum(data, 10, 18));

    if (command->isComplex()) {
        if (data.size() < Netstore::MIN_CMPLX_CMD_SIZE) {
            throw InvalidMessageException();
        }
        /* set param */
        command->setParam(parseNum(data, 18, 26));
    }

    /* set data */
    command->setData(std::vector(data.begin() + command->getDataStart(), data.end()));
    return command;
}

std::string CommandBuilder::parseCmd(const std::vector<char> &data) {
    /* get alpha chars from first 10 chars of data */
    std::string cmd{};
    for (auto ite = data.begin(); ite < data.begin() + 10; ++ite) {
        if (!isalpha(*ite)) break;
        cmd.push_back(*ite);
    }

    /* if the command is not recognised, throw exception */
    if (commandStrings.find(cmd) == commandStrings.end()) {
        throw InvalidMessageException();
    }
    return cmd;
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

Command SimpleGreetCommand::getResponse(const std::shared_ptr<IConnection> &connection, const std::shared_ptr<Node> &node) const {
    /* set answer parameters */
    ComplexGreetCommand command;
    command.setSeq(this->cmd_seq);
    command.setParam(node->getMemory());
    auto mcast = node->getGroup().getMCAST_ADDR();
    command.setData(vector<char>(mcast.begin(), mcast.end()));

    return command;
}

Command SimpleListCommand::getResponse(const std::shared_ptr<IConnection> &connection, const std::shared_ptr<Node> &node) const {
    MyListCommand command{};
    command.setSeq(this->cmd_seq);
    std::vector<char> list{};
    for (auto f : node->getFiles()) {
        list.insert(list.end(), f.begin(), f.end());
        list.push_back('\n');
    }
    command.setData(list);
    return command;
}


std::string Command::getRawData() const {
    string con = this->cmd;

    /* fill spare space with 0's */
    for (int i = this->cmd.size(); i < 10; ++i) {
        con += "0";
    }
    con += std::to_string(this->cmd_seq);
    return con;
}

std::string SimpleCommand::getRawData() const {
    return Command::getRawData() + this->data.data();
}

std::string ComplexCommand::getRawData() const {
    return Command::getRawData() + std::to_string(this->param) + this->data.data();
}
