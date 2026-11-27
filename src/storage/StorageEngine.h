//
// Created by david on 11/26/26.
//

#ifndef REDIS_STARTER_CPP_STORAGE_H
#define REDIS_STARTER_CPP_STORAGE_H
#include <expected>
#include <string>
#include <unordered_map>

#include "RedisObject.h"

enum class StorageError {
    WrongType
};

class StorageEngine {
private:

    std::unordered_map<std::string, RedisObject> elements_ {};

    static bool isExpired(const RedisObject& object) {
        return object.expiresAt.has_value() and std::chrono::system_clock::now() >= object.expiresAt.value();
    }

public:

    void set(const std::string& key, const std::string& value);
    void set(const std::string& key, const std::string& value, std::chrono::milliseconds);

    /**
     * Appends value to list_key list. If list doesn't exist, it creates a new one
     * @param list_key
     * @param value
     * @return the number of elements in the list
     */
    size_t rpush(const std::string& list_key, const std::string& value);

    /**
     * Retrieves elements from list_key list from lower to upper (inclusive).
     * If list doesn't exist, returns empty array.
     * Indices can be negative. Normalize indices before validating.
     * If start >= list.size(), return empty array
     * If stop >= list.size(), then stop = list.size()-1
     * If start > stop, return empty array
     * @param list_key
     * @param start
     * @param stop
     * @return list of items
     */
    std::vector<std::string> lrange(const std::string& list_key, std::ptrdiff_t start, std::ptrdiff_t stop);

    using GetResult = std::expected<std::optional<std::string_view>, StorageError>;
    GetResult get(const std::string &key);
};


#endif //REDIS_STARTER_CPP_STORAGE_H
