
#ifndef SIK2_COMMAND_H
#define SIK2_COMMAND_H

#include "err.h"
#include "Connection.h"
#include "group.h"

class ICommand {
public:
    virtual void execute(std::shared_ptr<IConnection> connection, std::shared_ptr<Node> node);

    virtual void setSeq(uint64_t seq);

    virtual void setParam(uint64_t param);

    virtual void setData(std::vector<char> data);

    virtual bool isComplex();

    /* returns position of first byte of data part of command */
    virtual uint32_t getDataStart();
};

class Command : public ICommand {
protected:
    std::string cmd;
    uint64_t cmd_seq{}; // bigendian
    std::vector<char> data{};

    static long getSeq() {
        return 0;
    }

public:
    explicit Command(std::string cmd) : cmd(std::move(cmd)), cmd_seq(getSeq()) {}

    void setSeq(uint64_t seq) override {
        this->cmd_seq = seq;
    }

    void setData(std::vector<char> dat) override {
        this->data = dat;
    }
};

class SimpleCommand : public Command {
public:
    explicit SimpleCommand(std::string cmd) : Command(std::move(cmd)) {};

    bool isComplex() override {
        return false;
    }

    void setParam(uint64_t par) override {}

    uint32_t getDataStart() override {
        return 18;
    }
};

class ComplexCommand : public Command {
    uint64_t param{}; // bigendian

public:
    explicit ComplexCommand(std::string cmd) : Command(std::move(cmd)) {};

    bool isComplex() override {
        return true;
    }

    void setParam(uint64_t par) override {
        this->param = par;
    }

    uint32_t getDataStart() override {
        return 26;
    }
};

class SimpleGreetCommand : public SimpleCommand {
public:
    SimpleGreetCommand() : SimpleCommand("HELLO") {};

    void execute(std::shared_ptr<IConnection> connection, std::shared_ptr<Node> node) override;
};

class ComplexGreetCommand : public ComplexCommand {
    std::vector<std::string> filenames{};
public:
    ComplexGreetCommand() : ComplexCommand("GOOD_DAY") {};

    void execute(std::shared_ptr<IConnection> connection, std::shared_ptr<Node> node) override {

    }
};

class SimpleListCommand : public SimpleCommand {
public:
    SimpleListCommand() : SimpleCommand("LIST") {};

    void execute(std::shared_ptr<IConnection> connection, std::shared_ptr<Node> node) override {

    }
};

class ComplexListCommand : public ComplexCommand {
public:
    ComplexListCommand() : ComplexCommand("MY_LIST") {}

    void execute(std::shared_ptr<IConnection> connection, std::shared_ptr<Node> node) override {

    }
};

class SimpleGetCommand : public SimpleCommand {
public:
    SimpleGetCommand() : SimpleCommand("GET") {}

    void execute(std::shared_ptr<IConnection> connection, std::shared_ptr<Node> node) override {

    }
};

class ComplexGetCommand : public ComplexCommand {
public:
    ComplexGetCommand() : ComplexCommand("CONNECT_ME") {}

    void execute(std::shared_ptr<IConnection> connection, std::shared_ptr<Node> node) override {

    }
};

class SimpleDeleteCommand : public SimpleCommand {
public:
    SimpleDeleteCommand() : SimpleCommand("DEL") {}

    void execute(std::shared_ptr<IConnection> connection, std::shared_ptr<Node> node) override {

    }
};

class RejectAddAnswer : public SimpleCommand {
public:
    RejectAddAnswer() : SimpleCommand("NO_WAY") {}

    void execute(std::shared_ptr<IConnection> connection, std::shared_ptr<Node> node) override {

    }
};

class AcceptAddAnswer : public ComplexCommand {
public:
    AcceptAddAnswer() : ComplexCommand("CAN_ADD") {}

    void execute(std::shared_ptr<IConnection> connection, std::shared_ptr<Node> node) override {

    }
};

class ComplexAddCommand : public ComplexCommand {
public:
    ComplexAddCommand() : ComplexCommand("ADD") {}

    void execute(std::shared_ptr<IConnection> connection, std::shared_ptr<Node> node) override {

    }
};

class CommandBuilder {
    inline static std::unordered_map<std::string, std::shared_ptr<Command>> commandStrings = {
            {"HELLO",      std::make_shared<SimpleGreetCommand>();},
            {"GOOD_DAY",   std::make_shared<ComplexGreetCommand>()},
            {"LIST",       std::make_shared<SimpleListCommand>()},
            {"MY_LIST",    std::make_shared<ComplexListCommand>()},
            {"GET",        std::make_shared<SimpleGetCommand>()},
            {"CONNECT_ME", std::make_shared<ComplexListCommand>()},
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
