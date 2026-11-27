//
// Created by david on 11/27/26.
//

#include "Connection.h"

#include <cerrno>
#include <sys/socket.h>

bool Connection::handle_read() {
    char tmp[4096];
    while (true) {
        ssize_t n = ::recv(sock_.fd(), tmp, sizeof(tmp), 0);
        if (n > 0) {
            read_buf_.append(tmp, static_cast<size_t>(n));
        } else if (n == 0) {
            return false; // Client closed connection cleanly
        } else {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) return true; // Drained for now
            return false; // Fatal socket error
        }
    }
}

bool Connection::handle_write() {
    while (write_offset_ < write_buf_.size()) {
        const char* data = write_buf_.data() + write_offset_;
        size_t remaining = write_buf_.size() - write_offset_;

        ssize_t n = ::send(sock_.fd(), data, remaining, 0);
        if (n > 0) {
            write_offset_ += static_cast<size_t>(n);
        } else {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) return true; // Kernel buffer full
            return false; // Fatal socket error
        }
    }

    // All queued bytes sent — reset buffer
    write_buf_.clear();
    write_offset_ = 0;
    return true;
}

void Connection::queue_response(std::string_view data) {
    write_buf_.append(data);
}