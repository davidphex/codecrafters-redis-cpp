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

struct StringHash {
    using is_transparent = void;
    size_t operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }
};

class StorageEngine {
private:

    std::unordered_map<std::string, RedisObject, StringHash, std::equal_to<>> elements_ {};

    static bool isExpired(const RedisObject& object) {
        return object.expiresAt.has_value() and std::chrono::system_clock::now() >= object.expiresAt.value();
    }

public:

    /**
     * Sets key value, with possibility of setting expiry time
     * If key exists, it overwrites the existing one
     * @param key
     * @param value
     */
    void set(std::string_view key, std::string_view value, std::optional<std::chrono::milliseconds> = std::nullopt);

    /**
     * Appends value to end of the list_key list. If list doesn't exist, it creates a new one
     * @param list_key
     * @param value
     * @return the number of elements in the list
     */
    size_t rpush(std::string_view list_key, std::string_view value);

    /**
     * Appends value to the beginning of the list_key list. If list doesn't exist, it creates a new one
     * @param list_key
     * @param value
     * @return the number of elements in the list
     */
    size_t lpush(std::string_view list_key, std::string_view value);

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
    std::vector<std::string> lrange(std::string_view list_key, std::ptrdiff_t start, std::ptrdiff_t stop);


    /**
     * Returns the length of the list. Returns 0 if list doesn't exist
     * @param list_key size_t
     * @return
     */
    size_t llen(std::string_view list_key);

    /**
     * Removes and returns the first element of the list.
     * If the list is empty or doesn't exist, it returns std::nullopt
     * @param list_key
     * @return
     */
    std::optional<std::string> lpop(std::string_view list_key);


    /**
     * Removes and returns the first numOfElements of the list.
     * @param list_key
     * @param numOfElements
     * @return
     */
    std::vector<std::string> lpop(std::string_view list_key, size_t numOfElements);

    /**
     * Returns the type of the value stored at the given key.
     * If key isn't found, returns std::nullopt
     * @param key
     * @return
     */
    std::optional<std::string_view> type(std::string_view key);

    using GetResult = std::expected<std::optional<std::string_view>, StorageError>;
    GetResult get(std::string_view key);
};


#endif //REDIS_STARTER_CPP_STORAGE_H
