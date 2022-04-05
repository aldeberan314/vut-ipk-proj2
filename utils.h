//
// Created by Andrej Hyros on 04/04/2022.
//
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>


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
    STOR,
    TOBE,
    ERROR
};


command_code hash_string(std::string &string);
void tokenize(std::string const &str, const char delim, std::vector<std::string> &out);

bool load_file(std::vector<std::string> &outvec, std::string fileName);