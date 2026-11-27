//
// Created by david on 11/26/26.
//

#include "Parser.h"

#include <iostream>

/*
 Check if the given command is valid (case-insensitive).
 Normalizes command to uppercase format.
 */
bool Parser::isValidCommand(std::string &command) {
    std::transform(command.begin(), command.end(), command.begin(), ::toupper);

    return commandMap.contains(command);
}

std::optional<std::vector<std::string>> Parser::parseCommand(std::string& buffer) {
    // Parse one RESP command from the front of the buffer:
    //   *<count>\r\n followed by <count> bulk strings $<len>\r\n<payload>\r\n
    // Only a fully received command is consumed; partial commands leave the
    // buffer untouched so the rest can be parsed once more bytes arrive.
    if (buffer.empty() || buffer[0] != '*') {
        return std::nullopt;
    }

    size_t pos = 1; // skip the leading '*'

    // Reads the \r\n-terminated line at pos as an integer, advancing pos
    const auto readIntLine = [&buffer, &pos](long long& out) -> bool {
        const size_t end = buffer.find("\r\n", pos);
        if (end == std::string::npos) return false;

        try {
            out = std::stoll(buffer.substr(pos, end - pos));
        } catch (const std::exception&) {
            return false;
        }

        pos = end + 2;
        return true;
    };

    long long numberOfElements {};
    if (!readIntLine(numberOfElements) || numberOfElements <= 0) {
        return std::nullopt;
    }

    std::vector<std::string> args;
    args.reserve(static_cast<size_t>(numberOfElements));

    for (long long i {}; i < numberOfElements; ++i) {
        if (pos >= buffer.size() || buffer[pos] != '$') {
            return std::nullopt;
        }
        ++pos; // skip the leading '$'

        long long elementLength {};
        if (!readIntLine(elementLength) || elementLength < 0) {
            return std::nullopt;
        }

        // payload + trailing \r\n must both be present
        if (buffer.size() < pos + static_cast<size_t>(elementLength) + 2) {
            return std::nullopt;
        }

        args.emplace_back(buffer, pos, static_cast<size_t>(elementLength));
        pos += static_cast<size_t>(elementLength) + 2;
    }

    buffer.erase(0, pos);

    return args;
}
