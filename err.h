#ifndef _ERR_
#define _ERR_

#include <exception>

/* wypisuje informacje o blednym zakonczeniu funkcji systemowej
i konczy dzialanie */
extern void syserr(const char *fmt, ...);

/* wypisuje informacje o bledzie i konczy dzialanie */
extern void fatal(const char *fmt, ...);

class InvalidMessageException : public std::exception {

};

class PartialSendException : public std::exception {

};

class InvalidInputException : public std::exception {

};
#endif
