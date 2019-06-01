#ifndef _ERR_
#define _ERR_

#include <exception>

/* wypisuje informacje o blednym zakonczeniu funkcji systemowej
i konczy dzialanie */
extern void syserr(const char *fmt, ...);

/* wypisuje informacje o bledzie i konczy dzialanie */
extern void fatal(const char *fmt, ...);

class InvalidMessageException : public std::exception {
public:
    const char *what() const noexcept override {
        return "Message command not recognized.\n";
    }

};

class PartialSendException : public std::exception {
public:
    const char *what() const noexcept override {
        return "Message was partially sent.\n";
    }
};

class InvalidInputException : public std::exception {
public:
    const char *what() const noexcept override {
        return "This command is invalid.\n";
    }


};
#endif
