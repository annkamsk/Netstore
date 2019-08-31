#include "FileSender.h"

void FileSender::init(std::string filename, int sock) {
    /* Open the file */
    if ((fd = open(filename.data(), O_RDONLY)) == -1) {
        throw FileException("Unable to open file " + filename);
    }
    bytesCount = 0;
    bytesSent = 0;
    buffer.clear();
    this->sock = sock;
}

int FileSender::handleSending() {
    if (!isSending) {
        return 2;     /* nothing to do */
    }
    /* if all bytes from buffer were sent */
    if (bytesSent == bytesCount) {
        /* Get one chunk of the file from disk */
        bytesCount = read(fd, buffer.data(), BUFSIZE);
        if (bytesCount == 0) {
            /* All done; close the file and socket. */
            close(fd);
            close(sock);
            isSending = false;
            return 1;
        }
        bytesSent = 0;
    }

    /* Send one chunk of the file */
    ssize_t bytes;
    if ((bytes = write(sock, buffer.data() + bytesSent, bytesCount - bytesSent)) < 0) {
        throw MessageSendException("Cannot send message.");
    }

    bytesSent += bytes;
    return 0;
}