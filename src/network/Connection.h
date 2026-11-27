//
// Created by david on 11/27/26.
//

#ifndef REDIS_STARTER_CPP_CONNECTION_H
#define REDIS_STARTER_CPP_CONNECTION_H
#include <string>

#include "Socket.h"


class Connection {
private:
    Socket sock_;
    std::string read_buf_;
    std::string write_buf_;
    size_t write_offset_ = 0;
    bool want_close_ = false;

public:
    explicit Connection(Socket sock) : sock_(std::move(sock)) {}

    [[nodiscard]] int fd() const { return sock_.fd(); }

    // Drains kernel buffer into read_buf_. Returns false if peer closed or fatal error.
    bool handle_read();

    // Flushes write_buf_ to kernel. Returns false on fatal error.
    bool handle_write();

    void queue_response(std::string_view data);
    [[nodiscard]] bool has_pending_writes() const { return write_offset_ < write_buf_.size(); }

    std::string& read_buffer() { return read_buf_; }
    void mark_for_close() { want_close_ = true; }
    [[nodiscard]] bool should_close() const { return want_close_; }
};


#endif //REDIS_STARTER_CPP_CONNECTION_H
