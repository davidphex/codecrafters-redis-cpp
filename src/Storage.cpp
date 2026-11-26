//
// Created by david on 11/26/26.
//

#include "Storage.h"

Storage& Storage::getInstance() {
    static Storage instance;
    return instance;
}

void Storage::set(const std::string &key, const std::string &value) {
    elements_[key] = value;
}

std::string Storage::get(const std::string &key) {
    if (elements_.contains(key)) {
        return elements_[key];
    }
    return nullBulkString;
}
