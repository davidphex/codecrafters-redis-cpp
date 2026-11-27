//
// Created by david on 11/27/26.
//

#include "Server.h"

#include <cerrno>
#include <csignal>
#include <stdexcept>
#include <sys/socket.h>

#include "Connection.h"

Server::Server(uint16_t port, OnDataCallback on_data, OnTickCallback on_tick)
    : listen_sock_(Socket::create_listener(port)),
      on_data_(std::move(on_data)),
      on_tick_(std::move(on_tick)) {
    if (!listen_sock_.valid()) {
        throw std::runtime_error("Failed to create listening socket");
    }
    ::signal(SIGPIPE, SIG_IGN); // Prevent crash if a client disconnects mid-write
    poller_.add(listen_sock_.fd(), /*want_read=*/true, /*want_write=*/false);
}

void Server::run() {
    while (true) {
        auto events = poller_.wait(/*timeout_ms=*/100);

        for (const auto& ev : events) {
            if (ev.fd == listen_sock_.fd()) {
                accept_new_clients();
            } else {
                handle_client_event(ev);
            }
        }

        sweep_closed_connections();

        if (on_tick_) {
            on_tick_();
        }
    }
}

void Server::accept_new_clients() {
    while (true) {
        int client_fd = ::accept(listen_sock_.fd(), nullptr, nullptr);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            break; // EAGAIN / EWOULDBLOCK: no more pending connections
        }

        Socket client_sock(client_fd);
        client_sock.set_nonblocking();

        auto conn = std::make_unique<Connection>(std::move(client_sock));
        poller_.add(client_fd, /*want_read=*/true, /*want_write=*/false);
        connections_.emplace(client_fd, std::move(conn));
    }
}

void Server::handle_client_event(const PollEvent& ev) {
    auto it = connections_.find(ev.fd);
    if (it == connections_.end()) return;
    Connection& conn = *it->second;

    if (ev.error) {
        conn.mark_for_close();
        return;
    }

    if (ev.readable) {
        if (!conn.handle_read()) {
            conn.mark_for_close();
            return;
        }
        if (on_data_) {
            on_data_(conn);
        }
    }

    if (ev.writable && !conn.should_close()) {
        if (!conn.handle_write()) {
            conn.mark_for_close();
            return;
        }
    }

    // Only subscribe to POLLOUT when we actually have bytes waiting to be sent
    poller_.update(ev.fd, /*want_read=*/true, /*want_write=*/conn.has_pending_writes());
}

void Server::sweep_closed_connections() {
    for (auto it = connections_.begin(); it != connections_.end();) {
        if (it->second->should_close()) {
            if (on_close_) {
                on_close_(it->first);
            }
            poller_.remove(it->first);
            it = connections_.erase(it); // ~Connection -> ~Socket -> ::close(fd)
        } else {
            ++it;
        }
    }
}

void Server::send_to_fd(int fd, std::string_view data) {
    const auto it = connections_.find(fd);

    if (it == connections_.end() || it->second->should_close()) {
        return; // connection is already gone
    }

    it->second->queue_response(data);

    // Make sure POLLOUT is armed so the queued bytes get flushed
    poller_.update(fd, /*want_read=*/true, /*want_write=*/it->second->has_pending_writes());
}