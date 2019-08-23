#include <utility>


#ifndef SIK2_MESSAGE_H
#define SIK2_MESSAGE_H

#include "err.h"

struct __attribute__((__packed__)) SimpleMessageRaw {
    char cmd[10];
    uint64_t seq;
    char data[1024];
};

struct __attribute__((__packed__)) ComplexMessageRaw {
    char cmd[10];
    uint64_t seq;
    uint64_t param;
    char data[1024];
};

class Message {
    std::string cmd;
    uint64_t cmd_seq{}; // big endian
    uint64_t param{}; // big endian
    std::vector<char> data;
    bool isComplex;

    static uint64_t getSeq() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis(1, 200);
        return dis(gen);
    }

public:
    Message(std::string cmd, bool isComplex) : cmd(std::move(cmd)), cmd_seq(getSeq()), isComplex(isComplex) {}

    bool isMessageComplex() const { return isComplex; }

    bool isOpeningTCP() const { return cmd == "CAN_ADD" || cmd == "CONNECT_ME"; }

    /* returns position of first byte of data part of message */
    uint32_t getDataStart() const { return isComplex ? 26 : 18; }

    std::shared_ptr<Message> getResponse();

    void completeMessage(int par, std::vector<char> dat);

    SimpleMessageRaw getSimpleMessageRaw();

    ComplexMessageRaw getComplexMessageRaw();

    /* setters, getters */

    void setSeq(uint64_t seq) {
        this->cmd_seq = seq;
    }

    void setData(std::vector<char> dat) {
        this->data = std::move(dat);
    }

    const std::vector<char> &getData() const {
        return data;
    }

    const std::string &getCmd() const {
        return cmd;
    }

    uint64_t getCmdSeq() const {
        return cmd_seq;
    }

    void setCmdSeq(uint64_t cmdSeq) {
        cmd_seq = cmdSeq;
    }

    virtual void setParam(uint64_t par) { this->param = par; }

    virtual uint64_t getParam() const { return param;}
};

class MessageBuilder {
    inline static std::vector<std::string> complexMessages = { "GOOD_DAY", "CONNECT_ME", "ADD", "CAN_ADD"};
    inline static std::vector<std::string> simpleMessages = { "HELLO", "LIST", "MY_LIST", "GET", "DEL", "NO_WAY"};
public:

    inline static std::unordered_map<std::string, std::string> responseMap = {
            {"HELLO", "GOOD_DAY"},
            {"LIST", "MY_LIST"},
            {"GET", "CONNECT_ME"},
            {"DEL", ""},
            {"ADD", "CAN_ADD|NO_WAY"}
    };

public:
    MessageBuilder() = default;

    static std::shared_ptr<Message> create(std::string cmd);

    static std::shared_ptr<Message> build(const std::vector<char> &data, uint64_t seq = 0);

    static bool isComplex(std::string cmd) {
        return std::any_of(complexMessages.begin(), complexMessages.end(), [&cmd](std::string s) { return s == cmd;});
    }

    static bool isSimple(std::string cmd) {
        return std::any_of(simpleMessages.begin(), simpleMessages.end(), [&cmd](std::string s) { return s == cmd;});
    }

private:
    static std::string parseCmd(const std::vector<char> &data);

    static uint64_t parseNum(const std::vector<char> &data, uint32_t from, uint32_t to);
};

#endif //SIK2_MESSAGE_H
