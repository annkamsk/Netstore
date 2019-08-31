#ifndef SIK2_FILESENDER_H
#define SIK2_FILESENDER_H

#include <bits/fcntl-linux.h>
#include <fcntl.h>
#include "err.h"

class FileSender {
    const static int BUFSIZE = 1024;
    int fd;               /* file being sent */
    int sock;
    std::vector<my_byte> buffer;    /* current chunk of file */
    int bytesCount;          /* bytes in buffer */
    int bytesSent;         /* bytes sent so far */
    bool isSending;

public:
    FileSender() : sock(-1), fd(-1),
                   buffer(std::vector<my_byte>(BUFSIZE, 0)), bytesCount(0), bytesSent(0), isSending(false) {}

    void init(std::string filename, int sock);

    int handleSending();

    void activate() {
        isSending = true;
    }

    int getSock() const {
        return sock;
    }

    bool getIsSending() const {
        return isSending;
    }

};


#endif //SIK2_FILESENDER_H
