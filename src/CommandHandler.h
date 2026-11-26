//
// Created by david on 11/26/26.
//

#ifndef REDIS_STARTER_CPP_COMMANDHANDLER_H
#define REDIS_STARTER_CPP_COMMANDHANDLER_H
#include <functional>
#include <iostream>
#include <string>

using CommandFunc = std::function<void(const std::vector <std::string>&)>;

class CommandHandler {
private:
    std::unordered_map<std::string, CommandFunc> commands {};
public:

    void addCommand(const std::string& command, CommandFunc func);

    void run(const std::vector<std::string>& tokens);


};


#endif //REDIS_STARTER_CPP_COMMANDHANDLER_H
