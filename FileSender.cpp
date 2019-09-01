#include "FileSender.h"

void FileSender::init(std::string filename, int sock, sockaddr_in clientAddr) {
    /* Open the file */
    if ((fd = open(filename.data(), O_RDONLY)) == -1) {
        throw FileException("Unable to open file " + filename);
    }
    this->sock = sock;
    this->clientAddr = clientAddr;
}

int FileSender::handleSending() {
    if (!isSending) {
        return 2;     /* nothing to do */
    }
    std::cerr << "sent: " << bytesSent << " of: " << bytesCount;
    /* if all bytes from buffer were sent */
    if (bytesSent == bytesCount) {
        /* Get one chunk of the file from disk */
        bytesCount = read(fd, buffer.data(), BUFSIZE);
        if (bytesCount == 0) {
            /* All done; close the file and socket. */
            close(fd);
            close(sock);
            std::cerr << "File uploaded.\n";
            return 1;
        } else if (bytesCount < 0) {
            return 4;
        }
        bytesSent = 0;
    }

    /* Send one chunk of the file */
    ssize_t bytes;
    if ((bytes = write(sock, buffer.data() + bytesSent, bytesCount - bytesSent)) < 0) {
        return 3;
    }
    std::cerr << "Sent " << bytes << " of data.\n";

    bytesSent += bytes;
    return 0;
}