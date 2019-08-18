#include <utility>

#include "Message.h"

std::shared_ptr<Message> MessageBuilder::build(const std::vector<char> &data, uint64_t seq) {
    if (data.size() < Netstore::MIN_SMPL_CMD_SIZE) {
        throw InvalidMessageException();
    }

    /* set cmd */
    std::string cmd = MessageBuilder::parseCmd(data);
    std::shared_ptr<Message> message;
    if (isComplex(cmd)) {
        message = std::make_shared<ComplexMessage>(cmd);
    } else {
        message = std::make_shared<SimpleMessage>(cmd);
    }

    /* set cmd_seq and, if applicable, check whether it's the same as awaited seq */
    message->setSeq(parseNum(data, 10, 18));
    if (seq != 0 && message->getCmdSeq() != seq) {
        throw WrongSeqException();
    }

    if (message->isComplex()) {
        if (data.size() < Netstore::MIN_CMPLX_CMD_SIZE) {
            throw InvalidMessageException();
        }
        /* set param */
        message->setParam(parseNum(data, 18, 26));
    }

    /* set data */
    message->setData(std::vector<char>(data.begin() + message->getDataStart(), data.end()));
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
    if (!isComplex(cmd) && !isSimple(cmd)) {
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
    std::string seq = std::vector<char>(data.begin() + from, data.begin() + to).data();
    return std::stoi(seq);
}

std::shared_ptr<Message> Message::getResponse() {
    std::string responseCmd = MessageBuilder::responseMap.at(this->getCmd());
    std::shared_ptr<Message> message;
    if (MessageBuilder::isComplex(responseCmd)) {
        message = std::make_shared<Message>(ComplexMessage(responseCmd));
    } else {
        message = std::make_shared<Message>(SimpleMessage(responseCmd));
    }
    message->setSeq(Message::getSeq());
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

void Message::completeMessage(int param, std::vector<char> data) {
    this->setParam(param);
    this->setData(std::move(data));
}

std::string SimpleMessage::getRawData() const {
    return Message::getRawData() + (this->data.empty() ? "" : this->data.data());
}

std::string ComplexMessage::getRawData() const {
    return Message::getRawData() + std::to_string(this->param) + this->data.data();
}

