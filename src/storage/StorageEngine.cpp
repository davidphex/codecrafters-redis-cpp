//
// Created by david on 11/26/26.
//

#include "StorageEngine.h"

#include <filesystem>


void StorageEngine::set(std::string_view key, std::string_view value, std::optional<std::chrono::milliseconds> expires_in ) {
    // no expiry time
    if (!expires_in.has_value()) {
        elements_.emplace(key, RedisObject {
            .value = value.data(),
            .expiresAt = std::nullopt
        });
    } else {
        elements_.emplace(key, RedisObject {
            .value = value.data(),
            .expiresAt = std::chrono::system_clock::now() + expires_in.value()
        });
    }
}

size_t StorageEngine::rpush(std::string_view list_key, std::string_view value) {
    const auto it = elements_.find(list_key);

    // the list doesn't exist, we need to create it
    if (it == elements_.end()) {
        elements_.emplace(
            list_key,
            RedisObject {
                .value = std::deque<std::string>{std::string(value)},
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


size_t StorageEngine::lpush(std::string_view list_key, std::string_view value) {
    const auto it = elements_.find(list_key);

    // the list doesn't exist, we need to create it
    if (it == elements_.end()) {
        elements_.emplace(
            list_key,
            RedisObject {
                .value = std::deque<std::string>{std::string(value)},
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

std::vector<std::string> StorageEngine::lrange(std::string_view list_key, std::ptrdiff_t start, std::ptrdiff_t stop) {
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

size_t StorageEngine::llen(std::string_view list_key) {
    const auto it = elements_.find(list_key);

    if (it == elements_.end()) {
        return 0; // key not found
    }

    if (const auto* list = std::get_if<RedisList>(&it->second.value)) {
        return list->size();
    }

    return 0;
}

std::optional<std::string> StorageEngine::lpop(std::string_view list_key) {
    const auto it = elements_.find(list_key);

    if (it == elements_.end()) {
        return std::nullopt; // key not found
    }

    if (auto* list = std::get_if<RedisList>(&it->second.value)) {

        if (list->empty()) {
            return std::nullopt;
        }

        auto firstElement = list->front();
        list->pop_front();
        return firstElement;
    }

    return std::nullopt;
}

std::vector<std::string> StorageEngine::lpop(std::string_view list_key, size_t numOfElements) {
    const auto it = elements_.find(list_key);

    if (it == elements_.end()) {
        return {}; // key not found
    }

    if (auto* list = std::get_if<RedisList>(&it->second.value)) {

        if (list->empty()) {
            return {};
        }

        if (list->size() < numOfElements) {
            numOfElements = list->size();
        }

        std::vector<std::string> result;
        for (size_t i{}; i<numOfElements; ++i) {
            result.emplace_back(list->front());
            list->pop_front();
        }

        return result;
    }

    return {};
}

std::optional<std::string_view> StorageEngine::type(std::string_view key) {
    auto it = elements_.find(key);

    if (it == elements_.end()) {
        return std::nullopt;
    }

    return get_type_name(it->second.value);
}

StorageEngine::GetResult StorageEngine::get(std::string_view key) {
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
