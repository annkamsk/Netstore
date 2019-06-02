#include <utility>

#ifndef _ERR_
#define _ERR_

#include <exception>
#include <string>

/* wypisuje informacje o blednym zakonczeniu funkcji systemowej
i konczy dzialanie */
extern void syserr(const char *fmt, ...);

/* wypisuje informacje o bledzie i konczy dzialanie */
extern void fatal(const char *fmt, ...);

class NetstoreException : public std::exception {
    std::string message;
public:
    NetstoreException() = default;
    NetstoreException(std::string message) : message(std::move(message)) {}
    const char *what() const noexcept override {
        return "Netstore exception.\n";
    }

    const char *details() const noexcept {
        return message.data();
    }
};

class InvalidMessageException : public NetstoreException {
public:
    const char *what() const noexcept override {
        return "Message command not recognized.\n";
    }

};

class PartialSendException : public NetstoreException {
public:
    const char *what() const noexcept override {
        return "Message was partially sent.\n";
    }
};

class InvalidInputException : public NetstoreException {
public:
    const char *what() const noexcept override {
        return "This command is invalid.\n";
    }

};

class WrongSeqException : public NetstoreException {
public:
    const char *what() const noexcept override {
        return "Packet with wrong seq.\n";
    }
};

class MessageSendException : public NetstoreException {
public:
    MessageSendException(std::string s) : NetstoreException(s) {}
    MessageSendException() = default;
    const char *what() const noexcept override {
        return "Packet with wrong seq.\n";
    }
};
#endif
