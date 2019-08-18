#include <utility>


#ifndef SIK2_MESSAGE_H
#define SIK2_MESSAGE_H

#include "err.h"
#include <random>
#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "Connection.h"

class Message {
private:

protected:
    std::string cmd;
    uint64_t cmd_seq{}; // TODO bigendian
    std::vector<char> data;

    static uint64_t getSeq() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis(1, UINT64_MAX);
        return dis(gen);
    }

public:
    explicit Message(std::string cmd) : cmd(std::move(cmd)), cmd_seq(getSeq()) {}

    void setSeq(uint64_t seq) {
        this->cmd_seq = seq;
    }

    void setData(std::vector<char> dat) {
        this->data = dat;
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

    const std::vector<char> &getData() const {
        return data;
    }

    virtual void setParam(uint64_t par) { (void)par; }

    virtual uint64_t getParam() const { return 0;}

    virtual bool isComplex() const { return false; }

    bool isOpeningTCP() const { return cmd == "CAN_ADD" || cmd == "CONNECT_ME"; }

    /* returns position of first byte of data part of message */
    virtual uint32_t getDataStart() const { return -1; }

    virtual std::string getRawData() const;

    std::shared_ptr<Message> getResponse();

    void completeMessage(int param, std::vector<char> data);
};

class SimpleMessage : public Message {
public:
    explicit SimpleMessage(const std::string& cmd) : Message(cmd) {};

    bool isComplex() const override {
        return false;
    }

    uint64_t getParam() const override { return 0; }

    uint32_t getDataStart() const override {
        return 18;
    }

    std::string getRawData() const override;
};

class ComplexMessage : public Message {
    uint64_t param{}; // bigendian

public:
    explicit ComplexMessage(const std::string& cmd) : Message(cmd) {};

    bool isComplex() const override {
        return true;
    }

    void setParam(uint64_t par) override {
        this->param = par;
    }

    uint64_t getParam() const override { return this->param; }

    uint32_t getDataStart() const override {
        return 26;
    }

    std::string getRawData() const override;
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
