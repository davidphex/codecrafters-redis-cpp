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

std::string Serializer::serializeArray(const std::vector<std::string> & elements) {
    std::ostringstream oss;

    if (elements.empty()) {
        oss << "*0\r\n";

        return oss.str();
    }

    oss << "*" << elements.size() << "\r\n";
    for (const auto & element : elements) {
        oss << serializeBulkString(element);
    }

    return oss.str();
}
