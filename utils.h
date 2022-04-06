//
// Created by Andrej Hyros on 04/04/2022.
//
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <chrono>


#define PRINT(x) std::cout << x << std::endl
#define PRINT2(x, y) std::cout << x << " " << y << std::endl

enum stream_type {
    ASCII,
    BINARY,
    CONTINUOUS
};


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
    STOP,
    SEND,
    ERROR
};


command_code hash_string(std::string &string);
void tokenize(std::string const &str, const char delim, std::vector<std::string> &out);

void get_rid_of_parents(std::filesystem::path &path);

bool load_file(std::vector<std::string> &outvec, std::string fileName);