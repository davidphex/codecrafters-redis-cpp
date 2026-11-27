//
// Created by david on 11/27/26.
//

#ifndef REDIS_STARTER_CPP_POLLER_H
#define REDIS_STARTER_CPP_POLLER_H
#include <vector>
#include <poll.h>
#pragma once

struct PollEvent {
    int fd;
    bool readable;
    bool writable;
    bool error;
};

class Poller {
public:
    void add(int fd, bool want_read, bool want_write);
    void update(int fd, bool want_read, bool want_write);
    void remove(int fd);

    std::vector<PollEvent> wait(int timeout_ms);

private:
    static short to_events(bool want_read, bool want_write);
    std::vector<pollfd> fds_;
};


#endif //REDIS_STARTER_CPP_POLLER_H
