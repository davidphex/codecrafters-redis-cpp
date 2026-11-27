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

std::string Serializer::serializeInteger(const int64_t integer) {
    std::ostringstream oss;

    // :[<-|+>]<value>\r\n

    oss << ":" << integer << "\r\n";

    return oss.str();
}
