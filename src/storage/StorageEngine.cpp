//
// Created by david on 11/26/26.
//

#include "StorageEngine.h"

#include <filesystem>

void StorageEngine::set(const std::string &key, const std::string &value) {
    elements_.emplace(key, RedisObject {
        .value = value,
        .expiresAt = std::nullopt
    });
}

void StorageEngine::set(const std::string &key, const std::string &value, std::chrono::milliseconds expiresIn) {
    elements_.emplace(key, RedisObject {
        .value = value,
        .expiresAt = std::chrono::system_clock::now() + expiresIn
    });
}

size_t StorageEngine::rpush(const std::string &list_key, const std::string &value) {
    const auto it = elements_.find(list_key);

    // the list doesn't exist, we need to create it
    if (it == elements_.end()) {
        elements_.emplace(
            list_key,
            RedisObject {
                .value = std::deque<std::string>{value},
                .expiresAt = std::nullopt
            }
        );
        return 1;
    }

    // when adding, check for it to be a list
    if (auto* list= std::get_if<RedisList>(&it->second.value)) {
        list->emplace_back(value);
        return list->size();
    }

    return 0;
}


size_t StorageEngine::lpush(const std::string &list_key, const std::string &value) {
    const auto it = elements_.find(list_key);

    // the list doesn't exist, we need to create it
    if (it == elements_.end()) {
        elements_.emplace(
            list_key,
            RedisObject {
                .value = std::deque<std::string>{value},
                .expiresAt = std::nullopt
            }
        );
        return 1;
    }

    // when adding, check for it to be a list
    if (auto* list= std::get_if<RedisList>(&it->second.value)) {
        list->emplace_front(value);
        return list->size();
    }

    return 0;
}

std::vector<std::string> StorageEngine::lrange(const std::string &list_key, std::ptrdiff_t start, std::ptrdiff_t stop) {
    // check if key exists
    const auto it = elements_.find(list_key);

    if (it==elements_.end()) {
        return {}; // key not found
    }

    // check if found object is a RedisList
    if (const auto* list = std::get_if<RedisList>(&it->second.value)) {

        if (list->empty()) {
            return {};
        }

        // normalize indices
        if (start < 0) {
            // if abs(start) is greater or equal than the list size, treat it as start = 0
            if (std::abs(start) >= list->size()) {
                start = 0;
            } else {
                start = static_cast<std::ptrdiff_t>(list->size()) + start;
            }
        }

        if (stop < 0) {
            stop = static_cast<std::ptrdiff_t>(list->size()) + stop;
        }

        if (start > stop) {
            return {};
        }

        if (start >= list->size()) {
            return {};
        }

        if (stop >= list->size()) {
            stop = static_cast<std::ptrdiff_t>(list->size())-1;
        }

        std::vector<std::string> result;
        for (std::ptrdiff_t i{start}; i<=stop; ++i) {
            result.emplace_back(list->at(i));
        }

        return result;

    }

    return {};
}

StorageEngine::GetResult StorageEngine::get(const std::string &key) {
    const auto it = elements_.find(key);

    if (it == elements_.end()) {
        return std::nullopt; // Key not found
    }

    if (isExpired(it->second)) {
        elements_.erase(it);
        return std::nullopt;
    }

    if (const auto* str = std::get_if<RedisString>(&it->second.value)) {
        return std::string_view{*str};
    }

    return std::unexpected(StorageError::WrongType);
}
