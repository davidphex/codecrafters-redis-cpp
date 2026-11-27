//
// Created by david on 11/27/26.
//

#ifndef REDIS_STARTER_CPP_COMMANDROUTER_H
#define REDIS_STARTER_CPP_COMMANDROUTER_H
#include <algorithm>
#include <deque>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

#include "storage/StorageEngine.h"

using CommandArgs = std::span<const std::string>;

/**
 * A command handler either returns an immediate reply, or std::nullopt when it
 * has taken over responsibility for replying later (BLPOP blocking a client).
 * The int parameter is the file descriptor of the requesting connection.
 */
using CommandHandler = std::function<std::optional<std::string>(StorageEngine&, CommandArgs, int)>;

struct CommandSpec {
    size_t min_args;
    size_t max_args;
    CommandHandler handler;
};

struct BlockedClient {
    int fd;
    std::optional<std::chrono::system_clock::time_point> deadline; // std::nullopt = timeout 0 (wait forever)
};

class CommandRouter {
public:
    // Pushes a reply to an arbitrary connection (owned by the Server)
    using DeliveryFunc = std::function<void(int fd, const std::string& reply)>;

    explicit CommandRouter(StorageEngine& storage_engine): storage_engine_(storage_engine) {
        register_defaults();
    }

    void set_delivery_callback(DeliveryFunc func) {
        delivery_ = std::move(func);
    }

    void register_command(std::string name, size_t min_args, size_t max_args, CommandHandler handler) {
        commands_.emplace(
            to_upper(name),
            CommandSpec{
                min_args,
                max_args,
                std::move(handler)
            }
        );
    }

    std::optional<std::string> execute(CommandArgs args, int fd) {
        if (args.empty()) {
            return "-ERR empty command\r\n";
        }

        // 1. Redis commands are case-insensitive ("get" == "GET")
        std::string cmd = to_upper(args[0]);

        auto it = commands_.find(cmd);
        if (it == commands_.end()) {
            return "-ERR unknown command '" + std::string(args[0]) + "'\r\n";
        }

        // 2. Centralized arity check
        const auto& spec = it->second;
        if (args.size() < spec.min_args || args.size() > spec.max_args) {
            return "-ERR wrong number of arguments for '" + cmd + "' command\r\n";
        }

        // 3. Dispatch to the handler
        return spec.handler(storage_engine_, args, fd);
    }

    /**
     * Expires blocked clients whose timeout has elapsed, replying with a null array.
     * Should be called regularly from the event loop.
     */
    void tick();

    /**
     * Drops any blocked-client entries for a connection that is going away,
     * so replies are never queued to a closed (or reused) fd.
     */
    void on_client_closed(int fd);

private:
    void register_defaults();

    /**
     * Serves clients blocked on key by popping elements from its list,
     * first-blocked-first-served, until the list or the waiters run out.
     */
    void serve_blocked(const std::string& key);

    void deliver(int fd, const std::string& reply);

    static std::string to_upper(std::string_view sv) {
        std::string out(sv);
        std::transform(out.begin(), out.end(), out.begin(), ::toupper);
        return out;
    }

    StorageEngine& storage_engine_;
    std::unordered_map<std::string, CommandSpec> commands_;
    std::unordered_map<std::string, std::deque<BlockedClient>> blocked_clients_;
    DeliveryFunc delivery_;
};


#endif //REDIS_STARTER_CPP_COMMANDROUTER_H
