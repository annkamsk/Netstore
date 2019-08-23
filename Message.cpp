#include <utility>

#include "Message.h"

std::shared_ptr<Message> MessageBuilder::build(const std::vector<char> &data, uint64_t seq) {
    if (data.size() < Netstore::MIN_SMPL_CMD_SIZE) {
        throw InvalidMessageException();
    }

    /* set cmd */
    std::string cmd = MessageBuilder::parseCmd(data);
    std::cerr << cmd;

    std::shared_ptr<Message> message = create(cmd);
    if (isComplex(cmd)) {
        ComplexMessageRaw *mess = (ComplexMessageRaw*) &data[0];
        message->setSeq(be64toh(mess->seq));
        message->setParam(be64toh(mess->param));
        // TODO set data
    } else {
        SimpleMessageRaw *mess = (SimpleMessageRaw*) &data[0];
        message->setSeq(be64toh(mess->seq));
        // TODO set data
    }
    std::cerr <<message->getCmdSeq() << "\n";

    /* if applicable, check whether the seq is the same as the awaited seq */
    if (seq != 0 && message->getCmdSeq() != seq) {
        throw WrongSeqException();
    }

    return message;
}

std::string MessageBuilder::parseCmd(const std::vector<char> &data) {
    /* get alpha chars from first 10 chars of data */
    std::string cmd;
    for (auto ite = data.begin(); ite < data.begin() + 10; ++ite) {
        if (!isalpha(*ite) && *ite != '_') break;
        cmd.push_back(*ite);
    }
    /* if the message is not recognised, throw exception */
    if (!isComplex(cmd) && !isSimple(cmd)) {
        throw InvalidMessageException();
    }
    return cmd;
}

std::shared_ptr<Message> MessageBuilder::create(std::string cmd) {
    return isComplex(cmd) ? std::make_shared<Message>(cmd, true) : std::make_shared<Message>(cmd, false);
}

std::shared_ptr<Message> Message::getResponse() {
    std::string responseCmd = MessageBuilder::responseMap.at(this->getCmd());
    auto message = MessageBuilder().create(responseCmd);
    message->setSeq(cmd_seq);
    return message;
}

void Message::completeMessage(int par, std::vector<char> dat) {
    this->setParam(par);
    this->setData(std::move(dat));
}

SimpleMessageRaw Message::getSimpleMessageRaw() {
    SimpleMessageRaw messageRaw{};
    for (int i = 0; i < 10; ++i) {
        messageRaw.cmd[i] = i < this->getCmd().size() ? this->getCmd().at(i) : 0;
    }
    messageRaw.seq = htobe64(this->getCmdSeq());

    return messageRaw;
}

ComplexMessageRaw Message::getComplexMessageRaw() {
    ComplexMessageRaw messageRaw{};
    for (int i = 0; i < 10; ++i) {
        messageRaw.cmd[i] = i < this->getCmd().size() ? this->getCmd().at(i) : 0;
    }
    messageRaw.seq = htobe64(this->getCmdSeq());

    return messageRaw;
}

