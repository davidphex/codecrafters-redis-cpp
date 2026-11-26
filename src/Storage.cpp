//
// Created by david on 11/26/26.
//

#include "Storage.h"

#include <iostream>

Storage& Storage::getInstance() {
    static Storage instance;
    return instance;
}

void Storage::set(const std::string &key, const std::string &value) {
    elements_.insert({key, Record(value)});
}

void Storage::set(const std::string &key, const std::string &value, std::chrono::milliseconds expiresIn) {
    elements_.insert({key, Record(value, expiresIn)});
}

std::optional<std::string> Storage::get(const std::string &key) {
    if (elements_.contains(key)) {

        const auto& record = elements_.at(key);

        if (std::chrono::system_clock::now() > record.expires_at()) {
            return std::nullopt;
        }

        return record.value();
    }
    return std::nullopt;
}
