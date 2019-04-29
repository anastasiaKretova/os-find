//
// Created by anastasia on 13.04.19.
//

#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <wait.h>

#include "shell.h"

void shell::printError(const std::string &message) {
    if (errno != 0) std::cerr << message << strerror(errno) << std::endl;
    else std::cerr << message << std::endl;
    std::cerr.flush();
}

void shell::run(std::vector<std::string> args, char *const *env) {
    pid_t pid = fork();
    if (pid == -1) {
        printError("Can't fork:  ");
    } else if (pid == 0) {
        std::vector<char*> argsChar;
        argsChar.reserve(args.size());
        for (const std::string& arg : args) {
            argsChar.push_back(const_cast<char*>(arg.data()));
        }
        argsChar.emplace_back(nullptr);

        if (execve(argsChar[0], argsChar.data(), env) == -1) {
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else {
        int exitCode;
        if (waitpid(pid, &exitCode, 0) == -1) {
            printError("Execution failed: ");
        } else {
            std::cout << "Executed. Return code: " << WEXITSTATUS(exitCode) << std::endl;
        }
    }
}


void shell::execute(const std::vector<std::string> &args) {
    run(args, nullptr);
}