//
// Created by david on 11/26/26.
//

#include "RESPParser.h"

#include <iostream>
#include <ostream>
#include <sstream>

RESPParser::RESPParser() {
}

RESPParser::~RESPParser() {
}

/*
 Check if the given command is valid (case-insensitive).
 Normalizes command to uppercase format.
 */
bool RESPParser::isValidCommand(std::string &command) {
    std::transform(command.begin(), command.end(), command.begin(), ::toupper);

    return commandMap.contains(command);
}

std::vector<std::string> RESPParser::parseCommand(const char &buffer) {
    std::vector<std::string> args;


    try {
        std::istringstream ss(&buffer);

        // check the number of elements
        size_t numberOfElements;

        // ignore the leading *
        if (ss.get(); ss >> numberOfElements && numberOfElements > 0) {
            if (ss.peek() == '\r') ss.get();
            if (ss.peek() == '\n') ss.get();
        } else {
            throw std::invalid_argument("Invalid number of elements " + std::to_string(numberOfElements));
        }

        // extract strings
        for (size_t i {0}; i < numberOfElements; ++i) {

            size_t elementLength {};
            std::string element {};

            // ignore the leading $
            if (ss.get(); ss >> elementLength and elementLength > 0) {
                if (ss.peek() == '\r') ss.get();
                if (ss.peek() == '\n') ss.get();
            } else {
                throw std::invalid_argument("Invalid element length " + std::to_string(elementLength));
            }

            if (ss >> element and element.length() == elementLength) {

                if (ss.peek() == '\r') ss.get();
                if (ss.peek() == '\n') ss.get();

                args.emplace_back(element);

            } else {
                throw std::invalid_argument("Invalid element " + element);
            }
        }

        // check if the first element is a valid command
        if (not isValidCommand(args.at(0))){
            throw std::invalid_argument("Invalid command " + args.at(0));
        }

    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return args;
}
