//
// Created by david on 11/27/26.
//

#ifndef REDIS_STARTER_CPP_SERVER_H
#define REDIS_STARTER_CPP_SERVER_H
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

#include "Poller.h"
#include "Socket.h"

class Connection;

class Server {
public:
    using OnDataCallback = std::function<void(Connection&)>;
    using OnTickCallback = std::function<void()>;
    using OnCloseCallback = std::function<void(int fd)>;

    Server(uint16_t port, OnDataCallback on_data, OnTickCallback on_tick);
    void run(); // Main event loop

    // Queues a reply on an arbitrary connection (used for deferred replies like BLPOP)
    void send_to_fd(int fd, std::string_view data);

    void set_on_close_callback(OnCloseCallback on_close) { on_close_ = std::move(on_close); }

private:
    void accept_new_clients();
    void handle_client_event(const PollEvent& ev);
    void sweep_closed_connections();

    Socket listen_sock_;
    Poller poller_;
    std::unordered_map<int, std::unique_ptr<Connection>> connections_;

    OnDataCallback on_data_; // Called after handle_read() succeeds
    OnTickCallback on_tick_; // Called periodically for active TTL expiration
    OnCloseCallback on_close_; // Called when a connection is dropped (for blocked-client cleanup)
};

#endif //REDIS_STARTER_CPP_SERVER_H
