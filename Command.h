
#ifndef SIK2_COMMAND_H
#define SIK2_COMMAND_H

#include "err.h"
#include "Connection.h"
#include "group.h"

class Command {
protected:
    std::string cmd;
    uint64_t cmd_seq{}; // bigendian
    std::vector<char> data{};

    static long getSeq() {
        return 0;
    }

public:
    explicit Command(std::string cmd) : cmd(std::move(cmd)), cmd_seq(getSeq()) {}

    void setSeq(uint64_t seq) {
        this->cmd_seq = seq;
    }

    void setData(std::vector<char> dat) {
        this->data = dat;
    }

    const std::string &getCmd() const {
        return cmd;
    }

    void setCmd(const std::string &cmd) {
        Command::cmd = cmd;
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

    /* returns position of first byte of data part of command */
    virtual uint32_t getDataStart() const { return -1; }

    virtual Command getResponse(const std::shared_ptr<IConnection> &connection, const std::shared_ptr<Node> &node) const { return Command(nullptr); }

    virtual std::string getRawData() const;

};

class SimpleCommand : public Command {
public:
    explicit SimpleCommand(std::string cmd) : Command(std::move(cmd)) {};

    bool isComplex() const override {
        return false;
    }

    uint64_t getParam() const override { return 0; }

    uint32_t getDataStart() const override {
        return 18;
    }

    std::string getRawData() const override;
};

class ComplexCommand : public Command {
    uint64_t param{}; // bigendian

public:
    explicit ComplexCommand(std::string cmd) : Command(std::move(cmd)) {};

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

class SimpleGreetCommand : public SimpleCommand {
public:
    SimpleGreetCommand() : SimpleCommand("HELLO") {};

    Command getResponse(const std::shared_ptr<IConnection> &connection, const std::shared_ptr<Node> &node) const override;
};

class ComplexGreetCommand : public ComplexCommand {
    std::vector<std::string> filenames{};
public:
    ComplexGreetCommand() : ComplexCommand("GOOD_DAY") {};
};

class SimpleListCommand : public SimpleCommand {
public:
    SimpleListCommand() : SimpleCommand("LIST") {}

    Command getResponse(const std::shared_ptr<IConnection> &connection, const std::shared_ptr<Node> &node) const override;
};

class MyListCommand : public SimpleCommand {
public:
    MyListCommand() : SimpleCommand("MY_LIST") {}
};

class SimpleGetCommand : public SimpleCommand {
public:
    SimpleGetCommand() : SimpleCommand("GET") {}
};

class ComplexGetCommand : public ComplexCommand {
public:
    ComplexGetCommand() : ComplexCommand("CONNECT_ME") {}
};

class SimpleDeleteCommand : public SimpleCommand {
public:
    SimpleDeleteCommand() : SimpleCommand("DEL") {}
};

class RejectAddAnswer : public SimpleCommand {
public:
    RejectAddAnswer() : SimpleCommand("NO_WAY") {}
};

class AcceptAddAnswer : public ComplexCommand {
public:
    AcceptAddAnswer() : ComplexCommand("CAN_ADD") {}
};

class ComplexAddCommand : public ComplexCommand {
public:
    ComplexAddCommand() : ComplexCommand("ADD") {}
};

class CommandBuilder {
    inline static std::unordered_map<std::string, std::shared_ptr<Command>> commandStrings = {
            {"HELLO",      std::make_shared<SimpleGreetCommand>()},
            {"GOOD_DAY",   std::make_shared<ComplexGreetCommand>()},
            {"LIST",       std::make_shared<SimpleListCommand>()},
            {"MY_LIST",    std::make_shared<MyListCommand>()},
            {"GET",        std::make_shared<SimpleGetCommand>()},
            {"CONNECT_ME", std::make_shared<MyListCommand>()},
            {"DEL",        std::make_shared<SimpleDeleteCommand>()},
            {"ADD",        std::make_shared<ComplexAddCommand>()},
            {"CAN_ADD",    std::make_shared<AcceptAddAnswer>()},
            {"NO_WAY",     std::make_shared<RejectAddAnswer>()}
    };

public:
    CommandBuilder() = default;

    std::shared_ptr<Command> build(const std::vector<char> &data);

private:
    std::string parseCmd(const std::vector<char> &data);

    uint64_t parseNum(const std::vector<char> &data, uint32_t from, uint32_t to);
};

#endif //SIK2_COMMAND_H
