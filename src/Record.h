//
// Created by david on 11/26/26.
//

#ifndef REDIS_STARTER_CPP_RECORD_H
#define REDIS_STARTER_CPP_RECORD_H
#include <chrono>
#include <string>
#include <utility>


class Record {
private:
    const std::string value_;
    const std::chrono::system_clock::time_point expiresAt_;
public:
    explicit Record(std::string  value) : value_(std::move(value)), expiresAt_(std::chrono::system_clock::time_point::max()) {}
    Record(std::string  value, const std::chrono::milliseconds expiresIn) :
        value_(std::move(value)),
        expiresAt_(std::chrono::system_clock::now() + expiresIn)
    {}

    [[nodiscard]] std::string value() const {
        return value_;
    }

    [[nodiscard]] std::chrono::system_clock::time_point expires_at() const {
        return expiresAt_;
    }
};


#endif //REDIS_STARTER_CPP_RECORD_H
