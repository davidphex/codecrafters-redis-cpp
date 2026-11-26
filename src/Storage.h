//
// Created by david on 11/26/26.
//

#ifndef REDIS_STARTER_CPP_STORAGE_H
#define REDIS_STARTER_CPP_STORAGE_H
#include <string>
#include <unordered_map>

#include "Record.h"


class Storage {
protected:
    static constexpr std::string nullBulkString = "$-1\r\n";

    static inline std::unordered_map<std::string, Record> elements_ {};

    Storage()= default;

    static Storage* instance_;
public:
    Storage(Storage&) = delete;
    void operator= (Storage&) = delete;

    static Storage& getInstance();

    static void set(const std::string& key, const std::string& value);
    static void set(const std::string& key, const std::string& value, std::chrono::milliseconds);
    static std::string get(const std::string& key);
};


#endif //REDIS_STARTER_CPP_STORAGE_H
