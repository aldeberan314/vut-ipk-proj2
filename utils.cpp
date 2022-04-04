//
// Created by Andrej Hyros on 04/04/2022.
//

#include "utils.h"



command_code hash_string(std::string &string) {
    //std::cout << "TO BE HASHED STRING: "<< string << "\n";
    if (string == "USER") return USER;
    if (string == "ACCT") return ACCT;
    if (string == "TYPE") return TYPE;
    if (string == "LIST") return LIST;
    if (string == "CDIR") return CDIR;
    if (string == "KILL") return KILL;
    if (string == "NAME") return NAME;
    if (string == "DONE") return DONE;
    if (string == "RETR") return RETR;
    if (string == "STOR") return STOR;
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


