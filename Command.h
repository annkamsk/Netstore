#ifndef SIK2_COMMAND_H


#define SIK2_COMMAND_H

#include "err.h"
#include "Connection.h"

class ICommand {
public:
    virtual void execute(std::shared_ptr<IConnection> connection) = 0;
    virtual void setSeq(uint64_t seq) = 0;
    virtual void setParam(uint64_t param) = 0;
    virtual void setData(std::shared_ptr<char> data) = 0;
    virtual bool isComplex() = 0;
};

class Command : public ICommand {
protected:
    std::string cmd;
    uint64_t cmd_seq{}; // bigendian
    uint64_t param{}; // bigendian
    std::shared_ptr<char> data{};
    static long getSeq() {
        return 0;
    }

public:
    explicit Command(std::string cmd) : cmd(std::move(cmd)), cmd_seq(getSeq()) {}
    void setSeq(uint64_t seq) override {
        this->cmd_seq = seq;
    }

    void setParam(uint64_t par) override {
        this->param = par;
    }

    void setData(std::shared_ptr<char> dat) override {
        this->data = std::move(dat);
    };
};

class GreetCommand : public Command {
public:

    explicit GreetCommand(std::string cmd) : Command(std::move(cmd)){}

    void execute(std::shared_ptr<IConnection> connection) override {

    }

    bool isComplex() override {
        return cmd != "HELLO";
    }

};

class ListCommand : public Command {
public:
    explicit ListCommand(std::string cmd) : Command(std::move(cmd)){}

    bool isComplex() override {
        return cmd != "LIST";
    }

private:
    void execute(std::shared_ptr<IConnection> connection) override {

    };
};

class GetCommand : public Command {
public:
    explicit GetCommand(std::string cmd) : Command(std::move(cmd)){}

    bool isComplex() override {
        return cmd != "LIST";
    }

private:
    void execute(std::shared_ptr<IConnection> connection) override {

    };
};

class CommandBuilder {
    inline static std::unordered_map<std::string, std::shared_ptr<Command>> commandStrings = {
            { "HELLO", std::make_shared<GreetCommand>("HELLO") },
            { "GOOD_DAY", std::make_shared<GreetCommand>("GOOD_DAY") }
    };

public:
    CommandBuilder() = default;
    std::shared_ptr<Command> build(const std::vector<char>& data);

private:

    std::string parseCmd(const std::vector<char> &data);

    uint64_t parseNum(const std::vector<char> &data, uint32_t from, uint32_t to);

    std::shared_ptr<char> parseData(const std::vector<char> &data, uint from);
};

#endif //SIK2_COMMAND_H
