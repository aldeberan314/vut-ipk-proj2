//
// Created by Andrej Hyros on 04/04/2022.
//

#include "utils.h"



command_code hash_string(std::string &string) {
    std::cout << "TO BE HASHED STRING: "<< string << "\n";
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
}