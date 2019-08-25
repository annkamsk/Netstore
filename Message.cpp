#include <utility>

#include "Message.h"

std::shared_ptr<Message> MessageBuilder::build(const std::vector<my_byte> &data, uint64_t seq, size_t size) {
    if (data.size() < Netstore::MIN_SMPL_CMD_SIZE) {
        throw InvalidMessageException();
    }

    /* set cmd */
    std::string cmd = MessageBuilder::parseCmd(data);

    std::shared_ptr<Message> message = create(cmd);

    /* set seq*/
    uint64_t msg_seq;
    memcpy(&msg_seq, &data[0] + 10, 8);
    message->setSeq(be64toh(msg_seq));

    /* if applicable, check whether the seq is the same as the awaited seq */
    if (seq != 0 && message->getCmdSeq() != seq) {
        throw WrongSeqException();
    }

    if (isComplex(cmd)) {
        /* set param */
        uint64_t param;
        memcpy(&param, &data[0] + 18, 8);
        message->setParam(be64toh(param));
    }

    /* set data */
    message->setData(std::vector<my_byte>(data.begin() + message->getDataStart(), data.begin() + size));
    std::cerr << "Message data size : " << message->getData().size() << "\n";

    return message;
}

std::string MessageBuilder::parseCmd(const std::vector<my_byte> &data) {
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

void Message::completeMessage(int par, std::vector<my_byte> dat) {
    this->setParam(par);
    this->setData(std::move(dat));
}

my_byte * Message::getRawData() {
    /* calculate size of message */
    size_t len = isComplex ? Netstore::MIN_CMPLX_CMD_SIZE : Netstore::MIN_SMPL_CMD_SIZE;
    size_t data_len = 0;
    for (; data_len < data.size() && data[data_len] != '\0'; ++data_len) {}

    auto *rawData = (my_byte *) malloc(len + data_len);
    /* save the command */
    for (size_t i = 0; i < 10; ++i) {
        rawData[i] = i < cmd.size() ? cmd.at(i) : 0;
    }
    /* save seq */
    uint64_t seqn = htobe64(cmd_seq);
    memcpy(rawData + 10, &seqn, sizeof(cmd_seq));
    /* save param if applicable */
    if (isComplex) {
        uint64_t paramn = htobe64(param);
        memcpy(rawData + 10 + sizeof(cmd_seq), &paramn, sizeof(param));
    }
    /* save data */
    memcpy(rawData + getDataStart(), data.data(), data_len);
    return rawData;
}

