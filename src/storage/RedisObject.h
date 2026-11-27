//
// Created by david on 11/26/26.
//

#ifndef REDIS_STARTER_CPP_RECORD_H
#define REDIS_STARTER_CPP_RECORD_H
#include <chrono>
#include <deque>
#include <string>
#include <variant>

using TimePoint = std::chrono::system_clock::time_point;

using RedisString = std::string;
using RedisList = std::deque<std::string>;

using RedisValue = std::variant<RedisString, RedisList>;

struct RedisObject {
    RedisValue value;
    std::optional<TimePoint> expiresAt;
};

#endif //REDIS_STARTER_CPP_RECORD_H
