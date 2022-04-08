//
// Created by Andrej Hyros on 08/04/2022.
//
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <csignal>

#include <chrono>
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <pcap.h>
#include <vector>
#include <fstream>
#include <filesystem>

#include "error.h"
#include "utils.h"

#define BUFFER_SIZE 1024

#ifndef SFTP_COMMUNICANT_H
#define SFTP_COMMUNICANT_H


class communicant {
    int m_socket;
    addrinfo m_hints;
    char m_ipaddr[INET6_ADDRSTRLEN];
    char m_buffer[BUFFER_SIZE];
public:

    void *get_in_addr(struct sockaddr *sa);

};


#endif //SFTP_COMMUNICANT_H
