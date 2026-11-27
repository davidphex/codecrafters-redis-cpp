//
// Created by david on 11/26/26.
//

#ifndef REDIS_STARTER_CPP_RESPSERIALIZER_H
#define REDIS_STARTER_CPP_RESPSERIALIZER_H
#include <optional>
#include <string>
#include <vector>


class Serializer {
private:
public:
    static constexpr std::string nullBulkString = "$-1\r\n";
    static constexpr std::string nullArray = "*-1\r\n";

    Serializer() = default;
    ~Serializer() = default;

    /*
        Serializes std::string into RESP bulk string
        https://redis.io/docs/latest/develop/reference/protocol-spec/#bulk-strings
     */
    static std::string serializeBulkString(const std::optional<std::string_view> &str);
    /*
        Serializes std::string into RESP simple string
        https://redis.io/docs/latest/develop/reference/protocol-spec/#simple-strings
     */
    static std::string serializeSimpleString(const std::string&);


    /**
     * https://redis.io/docs/latest/develop/reference/protocol-spec/#integers
     * encodes base-10, 64-bit integer into RESP integer string
     * @return encoded RESP integer string
     */
    static std::string serializeInteger(int64_t);


    /**
     * https://redis.io/docs/latest/develop/reference/protocol-spec/#arrays
     * If array is empty, *0\r\n
     * @return
     */
    static std::string serializeArray(const std::vector<std::string>&);

    /**
     * RESP2 null array, sent when BLPOP times out
     * https://redis.io/docs/latest/develop/reference/protocol-spec/#arrays
     * @return
     */
    static std::string serializeNullArray();
};

#endif //REDIS_STARTER_CPP_RESPSERIALIZER_H
