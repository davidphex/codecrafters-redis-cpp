//
// Created by david on 11/27/26.
//

#include "Socket.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        if (fd_ >= 0) ::close(fd_);
        fd_ = std::exchange(other.fd_, -1);
    }
    return *this;
}

bool Socket::set_nonblocking() {
    int flags = ::fcntl(fd_, F_GETFL, 0);
    if (flags == -1) return false;
    return ::fcntl(fd_, F_SETFL, flags | O_NONBLOCK) != -1;
}

bool Socket::set_reuse_addr() {
    int opt = 1;
    return ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != -1;
}

Socket Socket::create_listener(uint16_t port) {
    Socket sock(::socket(AF_INET, SOCK_STREAM, 0));
    if (!sock.valid()) return Socket{-1};

    sock.set_reuse_addr();
    sock.set_nonblocking();

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (::bind(sock.fd(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        return Socket{-1};
    }
    if (::listen(sock.fd(), SOMAXCONN) < 0) {
        return Socket{-1};
    }
    return sock;
}

