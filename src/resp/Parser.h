//
// Created by david on 11/26/26.
//

#ifndef REDIS_STARTER_CPP_RESPPARSER_H
#define REDIS_STARTER_CPP_RESPPARSER_H
#include <string>
#include <unordered_map>
#include <vector>

enum class Command {
    PING,
    ECHO,
    SET,
    GET,
    RPUSH
};

class Parser {
private:
    inline static const std::unordered_map<std::string, Command> commandMap = {
        {"PING", Command::PING},
        {"ECHO", Command::ECHO},
        {"SET", Command::SET},
        {"GET", Command::GET},
        {"RPUSH", Command::RPUSH},
    };

public:
    Parser();
    ~Parser();

    /*
     * Takes in RESP array, returns a vector of elements
     * https://redis.io/docs/latest/develop/reference/protocol-spec/#arrays
     */
    static std::vector<std::string> parseCommand (const char& buffer);

    static bool isValidCommand (std::string& command);
};


#endif //REDIS_STARTER_CPP_RESPPARSER_H
