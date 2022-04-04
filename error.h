//
// Created by Andrej Hyros on 03/04/2022.
//
#include <iostream>
#include <string>

#ifndef SFTP_ERROR_H
#define SFTP_ERROR_H

#define CONNECTION_ERROR 1
#define FILE_IO_ERROR 2


void error_call(int errcode, std::string errmsg, int lerrcode=0);


#endif //SFTP_ERROR_H
