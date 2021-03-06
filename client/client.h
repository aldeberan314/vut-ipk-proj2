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

#include "../utils/error.h"
#include "../utils/utils.h"
#include "argparser_client.h"

#define BUFFER_SIZE 1024

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
    std::string m_username;
    bool m_done = false;
    bool m_logged_in = false;
    bool m_send = false;
    bool m_stor_planned = false;
    std::vector<std::string> m_tquery;
    ArgParserClient *m_args;

public:
    sftpClient(ArgParserClient *args);

    /**
     * FUNKCIA JE PREVZANÁ Z INTERNETU
     * autor: Brian Hall
     * licencia: https://creativecommons.org/licenses/by-nc-nd/3.0/
     * zdroj: https://beej.us/guide/bgnet/html/
     */
    static void *get_in_addr(struct sockaddr *sa);

    /**
     * starts client
     *
    * ČASŤ FUNKCIE PREVZATÝ Z INTERNETU
    * autor: Brian Hall
    * licencia: https://creativecommons.org/licenses/by-nc-nd/3.0/
    * zdroj: https://beej.us/guide/bgnet/html/
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
    void send_file(std::string filename, int sockfd, int filesize) const;

    /**
     * parses users input. Important for checking if users requests are possible
     * @param user_input
     */
    void parse_user_input(const std::string& user_input);
};


