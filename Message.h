
#ifndef SIK2_MESSAGE_H
#define SIK2_MESSAGE_H

#include "err.h"
#include "group.h"

class Message {
protected:
    std::string cmd;
    uint64_t cmd_seq{}; // bigendian
    std::vector<char> data{};

    static long getSeq() {
        return 0;
    }

public:
    explicit Message(const std::string &cmd) : cmd(cmd), cmd_seq(getSeq()) {}

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

    virtual void setParam(uint64_t par) {}

    virtual uint64_t getParam() const { return 0;}

    virtual bool isComplex() const { return false; }

    /* returns position of first byte of data part of message */
    virtual uint32_t getDataStart() const { return -1; }

    virtual Message getResponse(const std::shared_ptr<Node> &node) const { return Message(nullptr); }

    virtual std::string getRawData() const;

};

class SimpleMessage : public Message {
public:
    explicit SimpleMessage(std::string cmd) : Message(std::move(cmd)) {};

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
    explicit ComplexMessage(std::string cmd) : Message(std::move(cmd)) {};

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

class SimpleGreetMessage : public SimpleMessage {
public:
    SimpleGreetMessage() : SimpleMessage("HELLO") {};

    Message getResponse(const std::shared_ptr<Node> &node) const override;
};

class ComplexGreetMessage : public ComplexMessage {
    std::vector<std::string> filenames{};
public:
    ComplexGreetMessage() : ComplexMessage("GOOD_DAY") {};
};

class SimpleListMessage : public SimpleMessage {
public:
    SimpleListMessage() : SimpleMessage("LIST") {}

    Message getResponse(const std::shared_ptr<Node> &node) const override;
};

class MyListMessage : public SimpleMessage {
public:
    MyListMessage() : SimpleMessage("MY_LIST") {}
};

class SimpleGetMessage : public SimpleMessage {
public:
    SimpleGetMessage() : SimpleMessage("GET") {}
};

class ComplexGetMessage : public ComplexMessage {
public:
    ComplexGetMessage() : ComplexMessage("CONNECT_ME") {}
};

class SimpleDeleteMessage : public SimpleMessage {
public:
    SimpleDeleteMessage() : SimpleMessage("DEL") {}
};

class RejectAddAnswer : public SimpleMessage {
public:
    RejectAddAnswer() : SimpleMessage("NO_WAY") {}
};

class AcceptAddAnswer : public ComplexMessage {
public:
    AcceptAddAnswer() : ComplexMessage("CAN_ADD") {}
};

class ComplexAddMessage : public ComplexMessage {
public:
    ComplexAddMessage() : ComplexMessage("ADD") {}
};

class MessageBuilder {
    inline static std::unordered_map<std::string, std::shared_ptr<Message>> messageStrings = {
            {"HELLO",      std::make_shared<SimpleGreetMessage>()},
            {"GOOD_DAY",   std::make_shared<ComplexGreetMessage>()},
            {"LIST",       std::make_shared<SimpleListMessage>()},
            {"MY_LIST",    std::make_shared<MyListMessage>()},
            {"GET",        std::make_shared<SimpleGetMessage>()},
            {"CONNECT_ME", std::make_shared<MyListMessage>()},
            {"DEL",        std::make_shared<SimpleDeleteMessage>()},
            {"ADD",        std::make_shared<ComplexAddMessage>()},
            {"CAN_ADD",    std::make_shared<AcceptAddAnswer>()},
            {"NO_WAY",     std::make_shared<RejectAddAnswer>()}
    };

public:
    MessageBuilder() = default;

    std::shared_ptr<Message> build(const std::vector<char> &data);

private:
    std::string parseCmd(const std::vector<char> &data);

    uint64_t parseNum(const std::vector<char> &data, uint32_t from, uint32_t to);
};

#endif //SIK2_MESSAGE_H
