//
// Created by david on 11/26/26.
//

#ifndef REDIS_STARTER_CPP_RESPSERIALIZER_H
#define REDIS_STARTER_CPP_RESPSERIALIZER_H
#include <string>


class Serializer {
private:
public:
    static constexpr std::string nullBulkString = "$-1\r\n";

    Serializer() = default;
    ~Serializer() = default;

    /*
        Serializes std::string into RESP bulk string
        https://redis.io/docs/latest/develop/reference/protocol-spec/#bulk-strings
     */
    static std::string serializeBulkString(const std::string&);
    /*
        Serializes std::string into RESP simple string
        https://redis.io/docs/latest/develop/reference/protocol-spec/#simple-strings
     */
    static std::string serializeSimpleString(const std::string&);
};

#endif //REDIS_STARTER_CPP_RESPSERIALIZER_H
