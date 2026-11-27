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

    using GetResult = std::expected<std::optional<std::string_view>, StorageError>;
    GetResult get(const std::string &key);
};


#endif //REDIS_STARTER_CPP_STORAGE_H
