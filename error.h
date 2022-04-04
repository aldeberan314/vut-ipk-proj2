//
// Created by Andrej Hyros on 03/04/2022.
//
#include <iostream>
#include <string>

#ifndef SFTP_ERROR_H
#define SFTP_ERROR_H


void error_call(int errcode, std::string errmsg, int lerrcode=0);


#endif //SFTP_ERROR_H
