//
// Created by david on 11/27/26.
//

#include "Poller.h"
#include <algorithm>
#include <cerrno>

short Poller::to_events(bool want_read, bool want_write) {
    short ev = 0;
    if (want_read)  ev |= POLLIN;
    if (want_write) ev |= POLLOUT;
    return ev;
}

void Poller::add(int fd, bool want_read, bool want_write) {
    fds_.push_back(pollfd{fd, to_events(want_read, want_write), 0});
}

void Poller::update(int fd, bool want_read, bool want_write) {
    for (auto& pfd : fds_) {
        if (pfd.fd == fd) {
            pfd.events = to_events(want_read, want_write);
            return;
        }
    }
}

void Poller::remove(int fd) {
    fds_.erase(
        std::remove_if(fds_.begin(), fds_.end(), [fd](const pollfd& p) { return p.fd == fd; }),
        fds_.end()
    );
}

std::vector<PollEvent> Poller::wait(int timeout_ms) {
    std::vector<PollEvent> ready;
    if (fds_.empty()) return ready;

    int count = ::poll(fds_.data(), fds_.size(), timeout_ms);
    if (count <= 0) return ready; // 0 = timeout, -1 = EINTR or error

    ready.reserve(static_cast<size_t>(count));
    for (const auto& pfd : fds_) {
        if (pfd.revents == 0) continue;
        ready.push_back(PollEvent{
            .fd       = pfd.fd,
            .readable = (pfd.revents & POLLIN) != 0,
            .writable = (pfd.revents & POLLOUT) != 0,
            .error    = (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0,
        });
    }
    return ready;
}