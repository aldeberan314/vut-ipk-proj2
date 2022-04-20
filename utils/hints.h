//
// Created by Andrej Hyros on 10/04/2022.
//
#include <string>
#include <map>

std::map<std::string, std::string> query_hints {
        {"USER", "-Invalid query, usage: \"USER <userid>\""},
        {"PASS", "-Invalid query, usage: \"PASS <password>\""},
        {"TYPE", "-Invalid query, usage: \"TYPE { A | B | C }\""},
        {"LIST", "-Invalid query, usage: \"LIST { F | V } directory-path\""},
        {"CDIR", "-Invalid query, usage: \"CDIR directory-path\""},
        {"KILL", "-Invalid query, usage: \"KILL <file_spec>\""},
        {"NAME", "-Invalid query, usage: \"NAME old-file-spec\""},
        {"DONE", "-Invalid query, usage: \"DONE\""},
        {"RETR", "-Invalid query, usage: \"RETR file-spec\""},
        {"STOR", "-Invalid query, usage: \"STOR { NEW | OLD | APP } file-spec\""},
        {"TOBE", "-Invalid query, usage: \"TOBE new-file-spec\""},
        {"STOP", "-Invalid query, usage: \"STOP\""},
        {"SEND", "-Invalid query, usage: \"SEND\""},
        {"SIZE", "-Invalid query, usage: \"SIZE <number-of-bytes-in-file>\""}
};