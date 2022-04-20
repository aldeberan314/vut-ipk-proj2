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
#include <algorithm>


#define PRINT(x) std::cout << x << std::endl
#define PRINT2(x, y) std::cout << x << " " << y << std::endl

typedef unsigned char BYTE;

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
    SIZE,
    ERROR
};

/**
 * Hashes input string
 * @param string
 * @return
 */
command_code code_string(std::string &string);

/**
 * Splits string into vector of substring by delimiter
 * @param str string to be splitted
 * @param delim delimeter
 * @param out output vector
 */
void tokenize(std::string const &str, const char delim, std::vector<std::string> &out);

/**
 * Deletes parent links ("..") from paths using regular expressions
 * @param path path to be cleansed
 */
void get_rid_of_parents(std::filesystem::path &path);

/**
 * Loads file into vector
 * @param outvec vectore that will store lines from file
 * @param fileName file
 * @return
 */
bool load_file(std::vector<std::string> &outvec, std::string fileName);

/**
 * Returns true if passed string represents a number
 * @param s source string
 * @return
 */
bool is_number(const std::string &s);

void make_str_upper(std::string &str);