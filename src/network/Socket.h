//
// Created by david on 11/27/26.
//

#ifndef REDIS_STARTER_CPP_SOCKET_H
#define REDIS_STARTER_CPP_SOCKET_H
#include <cstdint>
#include <unistd.h>
#include <utility>


class Socket {
private:
    int fd_ = -1;
public:
    explicit Socket(const int fd = -1) : fd_(fd) {}
    ~Socket() {if (fd_>=0) ::close(fd_);}

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept : fd_(std::exchange(other.fd_, -1)) {}
    Socket& operator=(Socket&& other) noexcept;

    [[nodiscard]] int fd() const { return fd_; }
    [[nodiscard]] bool valid() const { return fd_ >= 0; }

    bool set_nonblocking();
    bool set_reuse_addr();

    static Socket create_listener(std::uint16_t port);
};


#endif //REDIS_STARTER_CPP_SOCKET_H
