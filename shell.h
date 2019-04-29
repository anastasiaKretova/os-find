//
// Created by anastasia on 13.04.19.
//

#ifndef SEQUENCE_SHELL_H
#define SEQUENCE_SHELL_H



#include <string>
#include <vector>

class shell {
    static void printError(const std::string &message);

    static void run(std::vector<std::string> args, char *const *env);

public:
    static void execute(const std::vector<std::string>& args);
};



#endif //SEQUENCE_SHELL_H
