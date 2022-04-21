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
#include <net/if.h>
#include <csignal>

#include <chrono>
#include <thread>
#include <cstdlib>
#include <cstdio>
#include <pcap.h>
#include <vector>
#include <fstream>
#include <filesystem>
#include <regex>

#include "../utils/error.h"
#include "../utils/utils.h"
#include "argparser.h"


#ifndef SFTP_SERVER_H
#define SFTP_SERVER_H

#define BACKLOG 10
#define MAX_HOSTNAME_LEN 254
#define BUFFER_SIZE 1024


namespace fs = std::filesystem;

/**
 * This class represents sftp server impelmentation. It contains all important data as member variables.
 * Should have only one instance.
 * start() is used to start the server.
 */
class sftpServer {
    int m_socket; // connection socket
    int m_sock; // listening socket
    addrinfo m_hints;
    sockaddr_storage m_client_addr;
    std::vector<std::string> m_tquery;
    char m_ipaddr[INET6_ADDRSTRLEN];
    char m_hostname[MAX_HOSTNAME_LEN];
    char m_buffer[BUFFER_SIZE]; // buffer for recieving and sending data
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
    std::string m_port;
    stream_type m_stream_type;
    ArgParserServer *m_args;

public:
    sftpServer(ArgParserServer *args);


    /**
     * FUNKCIA JE PREVZANÁ Z INTERNETU
     * autor: Brian Hall
     * licencia: https://creativecommons.org/licenses/by-nc-nd/3.0/
     * zdroj: https://beej.us/guide/bgnet/html/
     */
    static int bind_to(addrinfo *ptr, int &yes,  addrinfo *servinfo);
    /**
     * FUNKCIA JE PREVZANÁ Z INTERNETU
     * autor: Brian Hall
     * licencia: https://creativecommons.org/licenses/by-nc-nd/3.0/
     * zdroj: https://beej.us/guide/bgnet/html/
     */
    static void *get_in_addr(struct sockaddr *sa);

    /**
     * stars servers communication interface
     *
    * ČASŤ FUNKCIE PREVZATÝ Z INTERNETU
    * autor: Brian Hall
    * licencia: https://creativecommons.org/licenses/by-nc-nd/3.0/
    * zdroj: https://beej.us/guide/bgnet/html/
    */
    void start();

    /**
     * accepts connection from client
     */
    void accept_connection();

    /**
     * closes connection to client
     */
    void close_connection() const;

    /**
     * implements infinite while loop for receiving requests and sending responses
     */
    void start_conversation();

    /**
     * parses clients query and call handling function
     */
    void parse_query();

    /**
     * these functions handles all the different queries client sends
     */
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

    /**
     * retrieves file from socket
     */
    void retrieve_file();

    /**
     * checks validity of multi-query requests (STOR-SIZE, RETR-SEND, NAME-TOBE...)
     */
    void check_tobe();

    /**
     * reads from socket
     * @return
     */
    int receive();

    /**
     * prepares message into buffer
     * @param msg
     */
    void load_buffer(std::string msg);

    /**
     * validates user passed from client by USER query
     * @param token
     * @param is_password
     * @return
     */
    bool is_valid_user(std::string token,  bool is_password);

    /**
     * checks if query has valid count of args
     * @param cnt
     * @param upto
     * @return
     */
    bool is_valid_count(int cnt, int upto = 0);

    /**
     * returns false if user is not logged in and prepares answer
     * @return
     */
    bool is_logged_in();

    /**
     * sends specified file
     * @param filename
     */
    void send_file(std::string filename);

    /**
     * resets relevant server settings for new connection
     */
    void reset_server_settings();
};




#endif //SFTP_SERVER_H
