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

#include <pcap.h>
#include <vector>
#include <fstream>
#include <filesystem>

#include "error.h"
#include "utils.h"


#ifndef SFTP_SERVER_H
#define SFTP_SERVER_H

#define PORT "5060"
#define BACKLOG 10
#define MAX_HOSTNAME_LEN 254
#define BUFFER_SIZE 500


namespace fs = std::filesystem;


class sftpServer {
    int m_socket;
    addrinfo m_hints;
    sockaddr_storage m_client_addr;
    std::vector<std::string> m_tquery;
    char m_ipaddr[INET6_ADDRSTRLEN];
    char m_hostname[MAX_HOSTNAME_LEN];
    char m_buffer[BUFFER_SIZE];
    bool m_logged_in = false;
    bool m_acc_sent = false;
    bool m_userid_sent = false;
    std::string m_userid;
    std::string m_password;
    fs::path m_wdir;


public:
    sftpServer();
    int bind_to(addrinfo *ptr, int &yes,  addrinfo *servinfo);
    void *get_in_addr(struct sockaddr *sa);
    void start();

    void closeConnection();

    void start_conversation();

    void parse_query();

    void cmd_user();

    void cmd_acct();

    void cmd_pass();

    void cmd_list();

    void load_buffer(std::string msg);

    bool is_valid_user(std::string userid);

};


#endif //SFTP_SERVER_H
