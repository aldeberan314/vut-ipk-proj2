//
// Created by Andrej Hyros on 03/04/2022.
//

#include "error.h"


void error_call(int errcode, std::string errmsg, int lerrcode) {
    std::cerr << errmsg << " " << lerrcode << "\n";
    exit(errcode);
}