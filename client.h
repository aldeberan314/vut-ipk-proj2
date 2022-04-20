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
#include "argparser_client.h"

#define BUFFER_SIZE 1024
#define LOCALHOST "127.0.0.1"
#define PORT "5060"

namespace fs = std::filesystem;

/**
 * Class representing sftp client
 */
class sftpClient/* : private communicant */{
    int m_socket;
    char m_buffer[BUFFER_SIZE];
    char m_ipaddr[INET6_ADDRSTRLEN];
    addrinfo m_hints;
    fs::path m_retr_filename;
    std::string m_send_filename;
    bool m_done = false;
    bool m_send = false;
    bool m_stor_planned = false;
    std::vector<std::string> m_tquery;
    ArgParserClient *m_args;

public:
    sftpClient(ArgParserClient *args);

    void *get_in_addr(struct sockaddr *sa);

    /**
     * starts client
     */
    void start();

    /**
     * start infite conversation loop
     */
    void start_conversation();

    /**
     * retrieves file from server
     * @param sockfd
     * @param filesize
     */
    void retrieve_file(int sockfd, size_t filesize);

    /**
     * sends file to server
     * @param filename
     * @param sockfd
     * @param filesize
     */
    void send_file(std::string filename, int sockfd, int filesize);

    /**
     * parses users input. Important for checking if users requests are possible
     * @param user_input
     */
    void parse_user_input(std::string user_input);
};


