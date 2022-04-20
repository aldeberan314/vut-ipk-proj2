//
// Created by Andrej Hyros on 03/04/2022.
//
#include <iostream>
#include <string>

#ifndef SFTP_ERROR_H
#define SFTP_ERROR_H

#define CONNECTION_ERROR 1
#define FILE_IO_ERROR 2
#define TRANSMISSION_ERROR 3
#define CMD_ARGUMENT_ERROR 4

/**
 * Prints error message to stderr and exit program with error code
 * @param errcode errorcode to exit program with
 * @param errmsg error message for stderr
 * @param lerrcode potential second error code for more detailed description of error (for ex. errno)
 */
void error_call(int errcode, std::string errmsg, int lerrcode=0);


#endif //SFTP_ERROR_H
