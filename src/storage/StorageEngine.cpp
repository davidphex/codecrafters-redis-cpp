//
// Created by david on 11/26/26.
//

#include "StorageEngine.h"

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
