//
// Created by Andrej Hyros on 04/04/2022.
//
#include <iostream>
#include <string>

#define PRINT(x) std::cout << x << std::endl
#define PRINT2(x, y) std::cout << x << " " << y << std::endl

enum command_code {
    USER,
    ACCT,
    PASS,
    TYPE,
    LIST,
    CDIR,
    KILL,
    NAME,
    DONE,
    RETR,
    STOR
};


command_code hash_string(std::string &string);