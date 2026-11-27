//
// Created by david on 11/26/26.
//

#ifndef REDIS_STARTER_CPP_RESPPARSER_H
#define REDIS_STARTER_CPP_RESPPARSER_H
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

enum class Command {
    PING,
    ECHO,
    SET,
    GET,
    RPUSH,
    LPUSH,
    LRANGE,
    LLEN,
    LPOP,
    BLPOP,
};

class Parser {
private:
    inline static const std::unordered_map<std::string, Command> commandMap = {
        {"PING", Command::PING},
        {"ECHO", Command::ECHO},
        {"SET", Command::SET},
        {"GET", Command::GET},
        {"RPUSH", Command::RPUSH},
        {"LPUSH", Command::LPUSH},
        {"LRANGE", Command::LRANGE},
        {"LLEN", Command::LLEN},
        {"LPOP", Command::LPOP},
        {"BLPOP", Command::BLPOP},
    };

public:
    Parser();
    ~Parser();

    /*
     * Takes in RESP array, returns a vector of elements.
     * On success, the parsed command is erased from the front of the buffer,
     * so it can be called repeatedly for pipelined commands.
     * Returns std::nullopt if the buffer doesn't hold a complete command yet.
     * https://redis.io/docs/latest/develop/reference/protocol-spec/#arrays
     */
    static std::optional<std::vector<std::string>> parseCommand (std::string& buffer);

    static bool isValidCommand (std::string& command);
};


#endif //REDIS_STARTER_CPP_RESPPARSER_H
