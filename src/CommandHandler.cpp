//
// Created by david on 11/26/26.
//

#include "CommandHandler.h"

void CommandHandler::addCommand(const std::string &command, CommandFunc func) {
    commands[command] = func;
}

void CommandHandler::run(const std::vector<std::string> &tokens) {
    if (tokens.empty()) return;

    const std::string& commandName = tokens.at(0);

    if (!commands.contains(commandName)) {
        std::cerr << "Command not found: " << commandName << std::endl;
        return;
    }

    commands[commandName](tokens);
}
