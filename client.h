//
// Created by Andrej Hyros on 08/04/2022.
//


//
// Created by Andrej Hyros on 02/04/2022.
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
#define LOCALHOST "127.0.0.1"
#define PORT "5060"

namespace fs = std::filesystem;

class sftpClient/* : private communicant */{
    int m_socket;
    char m_buffer[BUFFER_SIZE];
    char m_ipaddr[INET6_ADDRSTRLEN];
    addrinfo m_hints;
    fs::path m_retr_filename;

public:
    sftpClient();

    void *get_in_addr(struct sockaddr *sa);

    void start();

    void start_conversation();

    void retrieve_file(int sockfd, size_t filesize);

    void send_file(std::string filename, int sockfd, int filesize);

    void parse_user_input(std::string user_input);
};


