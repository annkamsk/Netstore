#include <utility>

#ifndef _ERR_
#define _ERR_

#include <exception>
#include <string>
#include <random>
#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <list>
#include <utility>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <functional>
#include <iostream>
#include <cstring>
#include <poll.h>
#include <experimental/filesystem>
#include <csignal>
#include <fstream>
#include <iterator>
#include <fcntl.h>


namespace fs = std::experimental::filesystem;

#define my_byte unsigned char

namespace Netstore {
    const static int MIN_SMPL_CMD_SIZE = 18;
    const static int MIN_CMPLX_CMD_SIZE = 26;
    /* max SIMPL_CMD size = 10 (cmd) + 8 (seq) + 256 (max filename length) */
    const static unsigned MAX_SMPL_CMD_SIZE = 274;

    const static unsigned BUFFER_LEN = 2048;
    const static unsigned MAX_UDP_PACKET_SIZE = 1024;

    inline std::string getKey(sockaddr_in addr) {
        auto ip = inet_ntoa(addr.sin_addr);
        return std::string(ip);
    }
}

/* wypisuje informacje o blednym zakonczeniu funkcji systemowej
i konczy dzialanie */
extern void syserr(const char *fmt, ...);

/* wypisuje informacje o bledzie i konczy dzialanie */
extern void fatal(const char *fmt, ...);

extern void info(const char *fmt, ...);

class NetstoreException : public std::exception {
    std::string message;
public:
    NetstoreException() = default;
    NetstoreException(std::string message) : message(std::move(message)) {}
    const char *what() const noexcept override {
        return "ERROR: Netstore exception. ";
    }

    const char *details() const noexcept {
        return message.data();
    }
};

class InvalidMessageException : public NetstoreException {
public:
    const char *what() const noexcept override {
        return "ERROR: Message command not recognized. ";
    }

};

class PartialSendException : public NetstoreException {
public:
    PartialSendException(std::string s) : NetstoreException(s) {}
    PartialSendException() = default;
    const char *what() const noexcept override {
        return "ERROR: Message was partially sent. ";
    }
};

class InvalidInputException : public NetstoreException {
public:
    const char *what() const noexcept override {
        return "ERROR: This command is invalid. ";
    }

};

class WrongSeqException : public NetstoreException {
public:
    const char *what() const noexcept override {
        return "ERROR: Packet with wrong seq. ";
    }
};

class MessageSendException : public NetstoreException {
public:
    MessageSendException(std::string s) : NetstoreException(s) {}
    MessageSendException() = default;
    const char *what() const noexcept override {
        return "ERROR: Message sending exception. ";
    }
};

class FileException : public NetstoreException {
public:
    FileException(std::string s) : NetstoreException(s) {}
    FileException() = default;
    const char *what() const noexcept override {
        return "ERROR: File exception. ";
    }
};


#endif
