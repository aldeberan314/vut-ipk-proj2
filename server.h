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


#ifndef SFTP_SERVER_H
#define SFTP_SERVER_H

#define PORT "5060"
#define BACKLOG 10
#define MAX_HOSTNAME_LEN 254
#define BUFFER_SIZE 1024


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
    bool m_userid_sent = false;
    bool m_password_sent = false;
    bool m_NAME = false;
    bool m_done = false;
    bool m_retr_planned = false;
    bool m_stor_planned = false;
    std::string m_userid;
    std::string m_password;
    std::string m_retrieved_filename;
    int m_retrieved_filesize;
    std::string m_stored_filename;
    int m_stored_filesize;
    fs::path m_wdir;
    fs::path m_path_to_be_renamed;
    stream_type m_stream_type;


public:
    sftpServer();
    int bind_to(addrinfo *ptr, int &yes,  addrinfo *servinfo);
    void *get_in_addr(struct sockaddr *sa);
    void start();

    void close_connection();

    void start_conversation();

    void parse_query();

    void cmd_user();

    void cmd_acct();

    void cmd_pass();

    void cmd_type();

    void cmd_list();

    void cmd_cdir();

    void cmd_kill();

    void cmd_name();

    void cmd_tobe();

    void cmd_done();

    void cmd_retr();

    void cmd_stop();

    void cmd_send();

    void cmd_stor();

    void cmd_size();




    void retrieve_file();

    void check_tobe();

    int receive();

    void load_buffer(std::string msg);

    bool is_valid_user(std::string token,  bool is_password);

    bool is_valid_count(int cnt, std::string msg, int upto = 0);

    void send_file(std::string filename);

};


#endif //SFTP_SERVER_H
