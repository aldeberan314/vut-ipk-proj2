//
// Created by Andrej Hyros on 04/04/2022.
//

#include "utils.h"
#include "error.h"



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
    std::regex regex("([^\\/]*\\/\\.\\.)(\\/|$)"); // regex for parents, example: folder1/../folder2/..
    path = regex_replace(path.string(), regex, "");
}

bool load_file(std::vector<std::string> &outvec, std::string fileName) {
    std::ifstream in(fileName.data()); // open stream
    if (!in) { // check if object is valid
        error_call(FILE_IO_ERROR, "\"Cannot open the File : \"" +  fileName);
        return false;
    }
    std::string str;
    while (std::getline(in, str)) { // read the next line from File untill it reaches the end.
        if (str.size() > 0)
            outvec.push_back(str);
    }
    in.close(); // close file
    return true;
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin(); // initialize iterator
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void make_str_upper(std::string &str) {
    std::transform(str.begin(), str.end(),str.begin(), ::toupper);
}