//
// Created by Andrej Hyros on 04/04/2022.
//

#include "utils.h"



command_code code_string(std::string &string) {
    if (string == "USER") return USER;
    if (string == "ACCT") return ACCT;
    if (string == "PASS") return PASS;
    if (string == "TYPE") return TYPE;
    if (string == "LIST") return LIST;
    if (string == "CDIR") return CDIR;
    if (string == "KILL") return KILL;
    if (string == "NAME") return NAME;
    if (string == "DONE") return DONE;
    if (string == "RETR") return RETR;
    if (string == "STOR") return STOR;
    if (string == "TOBE") return TOBE;
    if (string == "STOP") return STOP;
    if (string == "SEND") return SEND;
    if (string == "SIZE") return SIZE;
    return ERROR;
}

void tokenize(std::string const &str, const char delim, std::vector<std::string> &out)
{
    size_t start;
    size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }
}

void get_rid_of_parents(std::filesystem::path &path) {
    std::regex regex("([^\\/]*\\/\\.\\.)(\\/|$)");
    path = regex_replace(path.string(), regex, "");
}

bool load_file(std::vector<std::string> &outvec, std::string fileName) {
    std::ifstream in("userpass.txt");
    // Check if object is valid
    if (!in) {
        std::cerr << "Cannot open the File : " << fileName << std::endl;
        return false;
    }
    std::string str;
    // Read the next line from File untill it reaches the end.
    while (std::getline(in, str)) {
        // Line contains string of length > 0 then save it in vector
        if (str.size() > 0)
            outvec.push_back(str);
    }
    //Close The File
    in.close();
    return true;
}
// src: https://en.cppreference.com/w/cpp/filesystem/file_time_type
/*
std::string time_to_string(std::filesystem::file_time_type const& ftime) {
    std::time_t cftime = std::chrono::system_clock::to_time_t(std::chrono::file_clock::to_sys(ftime));
    return std::asctime(std::localtime(&cftime));
    std::chrono::
}
*/

