//
// Created by david on 11/26/26.
//

#include "Serializer.h"

#include <sstream>

std::string Serializer::serializeBulkString(const std::string& str) {
    std::ostringstream oss;

    oss << "$" << str.length() << "\r\n";
    oss << str << "\r\n";

    return oss.str();
}

std::string Serializer::serializeSimpleString(const std::string& str) {
    std::ostringstream oss;

    oss << "+" << str << "\r\n";

    return oss.str();
}
