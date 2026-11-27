//
// Created by david on 11/27/26.
//

#include "CommandRouter.h"

#include <chrono>
#include <iostream>
#include <ranges>

#include "resp/Serializer.h"

void CommandRouter::register_defaults() {
    register_command("PING", 1, 2, [](StorageEngine& storage_engine, CommandArgs args, int) {
      return Serializer::serializeSimpleString("PONG");
    });

    register_command("ECHO", 2, 2,[](StorageEngine& storage_engine, CommandArgs args, int) {
      return Serializer::serializeBulkString(args[1]);
    });

    register_command("SET", 3, 5, [](StorageEngine& storage_engine, CommandArgs args, int) {
      std::string_view key = args[1];
      std::string_view value = args[2];

      // check if record has expiry information
      if (args.size() == 5) {

        std::chrono::milliseconds expires_in {};

        const auto time_to_expire = std::stoi(std::string(args[4]));

        /* EX specifies the time in seconds, PX in milliseconds */
        if (args[3] == "EX")
          expires_in = std::chrono::milliseconds(time_to_expire * 1000);
        else if (args[3] == "PX")
          expires_in = std::chrono::milliseconds(time_to_expire);

        storage_engine.set(key.data(), value.data(), expires_in);
      } else {

        // the record won't expire
        storage_engine.set(key.data(), value.data());
      }

      return Serializer::serializeSimpleString("OK");
    });

    register_command("GET", 2, 2,[](StorageEngine& storage_engine, CommandArgs args, int) {

      auto result = storage_engine.get(args[1].data());

      if (!result.has_value())
        return Serializer::serializeBulkString(std::nullopt);

      if (!result->has_value())
        return Serializer::serializeBulkString(std::nullopt);

      return Serializer::serializeBulkString(result.value());

    });

    register_command("RPUSH", 3, 99, [this](StorageEngine& storage_engine, CommandArgs args, int) {
      const std::string list_key(args[1]);

      size_t lastSize{};

      for (auto iterator = args.begin() + 2; iterator != args.end(); ++iterator) {
        lastSize = storage_engine.rpush(list_key, *iterator);
      }

      // Clients blocked in BLPOP are served before the elements stay in the list
      serve_blocked(list_key);

      return Serializer::serializeInteger(static_cast<int64_t>(lastSize));
    });

  register_command("LPUSH", 3, 99, [this](StorageEngine& storage_engine, CommandArgs args, int) {
    const std::string list_key(args[1]);

    size_t lastSize{};

    for (auto iterator = args.begin() + 2; iterator != args.end(); ++iterator) {
      lastSize = storage_engine.lpush(list_key, *iterator);
    }

    serve_blocked(list_key);

    return Serializer::serializeInteger(static_cast<int64_t>(lastSize));
  });

  register_command("LRANGE", 4, 4, [](StorageEngine& storage_engine, CommandArgs args, int) {
      const auto& list_key = args[1];

      const auto& start = std::stoi(std::string(args[2]));
      const auto& stop = std::stoi(std::string(args[3]));

      const auto listItems = storage_engine.lrange(
        list_key, start, stop
      );

      return Serializer::serializeArray(listItems);
  });

    register_command("LLEN", 2, 2, [](StorageEngine& storage_engine, CommandArgs args, int) {
      const auto& list_key = args[1];

      return Serializer::serializeInteger(static_cast<int64_t>(storage_engine.llen(list_key)));
    });

    register_command("LPOP", 2, 3, [](StorageEngine& storage_engine, CommandArgs args, int) {
      const auto& list_key = args[1];

      std::string response {};

      if (args.size() == 3) {
        const auto& num_of_elements = std::stoi(std::string(args[2]));

        const auto& multiple_elements = storage_engine.lpop(list_key, num_of_elements);

        return Serializer::serializeArray(multiple_elements);
      }

      const auto single_element = storage_engine.lpop(list_key);

      return Serializer::serializeBulkString(single_element);
    });

    /**
     * BLPOP key timeout
     * The timeout is a float in seconds; 0 blocks forever.
     * If the list has an element, replies immediately with [key, element].
     * Otherwise the client is blocked until a push delivers an element,
     * or the timeout elapses and a null array is sent.
     */
    register_command("BLPOP", 3, 3, [this](StorageEngine& storage_engine, CommandArgs args, int fd) -> std::optional<std::string> {
      const std::string list_key(args[1]);

      double timeout {};
      try {
        timeout = std::stod(std::string(args[2]));
      } catch (const std::exception&) {
        return std::string("-ERR timeout is not a float\r\n");
      }

      if (timeout < 0) {
        return std::string("-ERR timeout is negative\r\n");
      }

      // the list already has an element, pop it right away
      if (const auto element = storage_engine.lpop(list_key)) {
        return Serializer::serializeArray({list_key, *element});
      }

      std::optional<std::chrono::system_clock::time_point> deadline {};

      // timeout 0 means wait forever
      if (timeout > 0) {
        deadline = std::chrono::system_clock::now()
          + std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(timeout));
      }

      blocked_clients_[list_key].push_back(BlockedClient{ .fd = fd, .deadline = deadline });

      // no immediate reply, the response is delivered by serve_blocked or tick
      return std::nullopt;
    });
}

void CommandRouter::serve_blocked(const std::string& key) {
    const auto it = blocked_clients_.find(key);

    if (it == blocked_clients_.end()) {
        return;
    }

    auto& waiters = it->second;

    // first-blocked-first-served
    while (!waiters.empty()) {
        const auto element = storage_engine_.lpop(key);

        if (!element.has_value()) {
            break; // list is empty again, the remaining clients stay blocked
        }

        const BlockedClient waiter = waiters.front();
        waiters.pop_front();

        deliver(waiter.fd, Serializer::serializeArray({key, *element}));
    }

    if (waiters.empty()) {
        blocked_clients_.erase(it);
    }
}

void CommandRouter::tick() {
    const auto now = std::chrono::system_clock::now();

    for (auto key_it = blocked_clients_.begin(); key_it != blocked_clients_.end();) {
        auto& waiters = key_it->second;

        for (auto waiter_it = waiters.begin(); waiter_it != waiters.end();) {
            if (waiter_it->deadline.has_value() and now >= waiter_it->deadline.value()) {
                deliver(waiter_it->fd, Serializer::serializeNullArray());
                waiter_it = waiters.erase(waiter_it);
            } else {
                ++waiter_it;
            }
        }

        if (waiters.empty()) {
            key_it = blocked_clients_.erase(key_it);
        } else {
            ++key_it;
        }
    }
}

void CommandRouter::on_client_closed(int fd) {
    for (auto key_it = blocked_clients_.begin(); key_it != blocked_clients_.end();) {
        auto& waiters = key_it->second;

        std::erase_if(waiters, [fd](const BlockedClient& client) { return client.fd == fd; });

        if (waiters.empty()) {
            key_it = blocked_clients_.erase(key_it);
        } else {
            ++key_it;
        }
    }
}

void CommandRouter::deliver(int fd, const std::string& reply) {
    if (delivery_) {
        delivery_(fd, reply);
    } else {
        std::cerr << "No delivery callback, dropped reply for fd " << fd << std::endl;
    }
}
