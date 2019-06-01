#include "Message.h"

std::shared_ptr<Message> MessageBuilder::build(const std::vector<char> &data) {
    if (data.size() < Netstore::MIN_SMPL_CMD_SIZE) {
        throw InvalidMessageException();
    }

    /* set cmd */
    std::string cmd = this->parseCmd(data);
    auto message = messageStrings.at(cmd);

    /* set cmd_seq */
    message->setSeq(parseNum(data, 10, 18));

    if (message->isComplex()) {
        if (data.size() < Netstore::MIN_CMPLX_CMD_SIZE) {
            throw InvalidMessageException();
        }
        /* set param */
        message->setParam(parseNum(data, 18, 26));
    }

    /* set data */
    message->setData(std::vector(data.begin() + message->getDataStart(), data.end()));
    return message;
}

std::string MessageBuilder::parseCmd(const std::vector<char> &data) {
    /* get alpha chars from first 10 chars of data */
    std::string cmd;
    for (auto ite = data.begin(); ite < data.begin() + 10; ++ite) {
        if (!isalpha(*ite)) break;
        cmd.push_back(*ite);
    }

    /* if the message is not recognised, throw exception */
    if (messageStrings.find(cmd) == messageStrings.end()) {
        throw InvalidMessageException();
    }
    return cmd;
}

uint64_t MessageBuilder::parseNum(const std::vector<char> &data, uint32_t from, uint32_t to) {
    /* check if number was given */
    if (!std::all_of(data.begin() + from, data.begin() + to,
                     [](char c) { return isdigit(c); })) {
        throw InvalidMessageException();
    }
    std::string seq = std::vector(data.begin() + from, data.begin() + to).data();
    return std::stoi(seq);
}

Message SimpleGreetMessage::getResponse(const std::shared_ptr<Node> &node) const {
    /* set answer parameters */
    ComplexGreetMessage message;
    message.setSeq(this->cmd_seq);
    message.setParam(node->getMemory());
    auto mcast = node->getConnection()->getMcast();
    message.setData(std::vector<char>(mcast.begin(), mcast.end()));

    return message;
}

Message SimpleListMessage::getResponse(const std::shared_ptr<Node> &node) const {
    MyListMessage message{};
    message.setSeq(this->cmd_seq);
    std::vector<char> list{};
    for (auto f : node->getFiles()) {
        list.insert(list.end(), f.begin(), f.end());
        list.push_back('\n');
    }
    message.setData(list);
    return message;
}


std::string Message::getRawData() const {
    std::string con(this->cmd);
    /* fill spare space with 0's */
    for (int i = con.size(); i < 10; ++i) {
        con.push_back(0);
    }
    con += std::to_string(this->cmd_seq);
    return con;
}

std::string SimpleMessage::getRawData() const {
    return Message::getRawData() + (this->data.empty() ? "" : this->data.data());
}

std::string ComplexMessage::getRawData() const {
    return Message::getRawData() + std::to_string(this->param) + this->data.data();
}
