//
// Created by david on 11/26/26.
//

#ifndef REDIS_STARTER_CPP_RECORD_H
#define REDIS_STARTER_CPP_RECORD_H
#include <chrono>
#include <deque>
#include <string>
#include <variant>

#include "Stream.h"

using TimePoint = std::chrono::system_clock::time_point;

using RedisString = std::string;
using RedisList = std::deque<std::string>;
using RedisStream = Stream;

using RedisValue = std::variant<RedisString, RedisList, RedisStream>;

constexpr std::array<std::string_view, std::variant_size_v<RedisValue>> RedisValueTypeNames = {
    "string",
    "list",
    "stream",
};

constexpr std::string_view get_type_name(const RedisValue& v) {
    if (v.valueless_by_exception()) return "invalid";
    return RedisValueTypeNames[v.index()];
}

struct RedisObject {
    RedisValue value;
    std::optional<TimePoint> expiresAt;
};

#endif //REDIS_STARTER_CPP_RECORD_H
