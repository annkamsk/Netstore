#ifndef SIK2_FILESENDER_H
#define SIK2_FILESENDER_H

#include "err.h"

class FileSender {
    const static int BUFSIZE = 1024;
    int fd;               /* file being sent */
    int sock;
    sockaddr_in clientAddr{};
    std::string filename{};
    std::vector<my_byte> buffer;    /* current chunk of file */
    int bytesCount;          /* bytes in buffer */
    int bytesSent;         /* bytes sent so far */
    bool isSending;

public:
    FileSender() : fd(-1), sock(-1),
                   buffer(std::vector<my_byte>(BUFSIZE, 0)), bytesCount(0), bytesSent(0), isSending(false) {}

    void init(std::string filename, int sock, sockaddr_in clientAddr);

    int handleSending();

    int getSock() const {
        return sock;
    }

    bool getIsSending() const {
        return isSending;
    }

    sockaddr_in getClientAddr() const {
        return clientAddr;
    }

    std::string getFilename() const {
        return filename;
    }

};


#endif //SIK2_FILESENDER_H
