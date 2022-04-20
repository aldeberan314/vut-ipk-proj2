//
// Created by Andrej Hyros on 08/04/2022.
//

#include "client.h"


sftpClient::sftpClient(ArgParserClient *args) {
    memset(&m_hints, 0, sizeof(m_hints));
    memset(m_buffer, 0, BUFFER_SIZE);
    m_hints.ai_addr = AF_UNSPEC;
    m_hints.ai_socktype = SOCK_STREAM;
    m_args = args;
}

void *sftpClient::get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

void sftpClient::start() {
    int gai_r;
    addrinfo *servinfo, *p;
    char *ip = m_args->m_hArg.data();
    char *port = m_args->m_pArg.data();

    gai_r = getaddrinfo(ip, port, &m_hints, &servinfo);
    if(gai_r != 0) error_call(CONNECTION_ERROR, "gai error", errno);

    for(p = servinfo; p != NULL; p = p->ai_next) {
        m_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(m_socket == -1) {
            PRINT("socket error, continuing...");
            continue;
        }
        if(connect(m_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(m_socket);
            PRINT("connect error, continuing...");
            continue;
        }
        break;
    }
    if (p == NULL) error_call(CONNECTION_ERROR, "client: failed to connect, returning", errno);

    inet_ntop(p->ai_family, get_in_addr((sockaddr*)p->ai_addr), m_ipaddr, sizeof m_ipaddr);
    //printf("client: connecting to %s\n", m_ipaddr);

    freeaddrinfo(servinfo);

    start_conversation();

    close(m_socket);
}

void sftpClient::start_conversation() {
    int rec_bytes, filesize;
    std::string user_input;
    bool retr_sent = false;

    if((rec_bytes = recv(m_socket, m_buffer, BUFFER_SIZE-1, 0)) == -1) { // receive welcome message
        PRINT("recv error, exiting...");
        exit(0);
    }
    PRINT(m_buffer); // print welcome message
    while(true) {
        m_buffer[rec_bytes] = '\0';
        memset(m_buffer, 0, BUFFER_SIZE); // reset buffer to zeros

        std::getline(std::cin, user_input); // read user input

        // parse user input
        parse_user_input(user_input);


        memcpy(m_buffer, user_input.data(), user_input.length()); // load input to buffer
        send(m_socket, m_buffer, user_input.length(), 0); // send it
        memset(m_buffer, 0, BUFFER_SIZE); // prepare buffer for read
        make_str_upper(m_tquery.front());
        if(m_tquery.front() == "DONE") break;

        if(m_tquery.front() == "SIZE") {
            if(m_stor_planned && m_tquery.size() == 2 && is_number(m_tquery[1])) {
                send_file(m_send_filename, m_socket, atoi(user_input.data() + 4));
            }
        }

        if(m_tquery.front() == "STOP") retr_sent = false;

        if(m_tquery.front() == "SEND") {
            if(retr_sent) {
                retrieve_file(m_socket, filesize);
                retr_sent = false;
            }
        }

        if((rec_bytes = recv(m_socket, m_buffer, BUFFER_SIZE-1, 0)) == -1) { // read from server
            error_call(CONNECTION_ERROR, "Recv error, exiting", errno);
        }

        if(m_tquery.front() == "RETR") {
            retr_sent = true;
            filesize = atoi(m_buffer);
        }

        if(std::string(m_buffer) == "!" + m_username + " logged in") {
            m_logged_in = true;
        }

        // print server msg
        PRINT(m_buffer);
        memset(m_buffer, 0, BUFFER_SIZE);

        m_tquery.clear();
    }
}

void sftpClient::retrieve_file(int sockfd, size_t filesize) {

    char buffer[BUFFER_SIZE]; // buffer for file data
    bzero(buffer, BUFFER_SIZE); // reset buffer
    int remainingData = filesize; // obvious
    ssize_t len;
    FILE *fp; // file handle
    std::string path_to_file = m_args->m_fArg + "/" + m_retr_filename.string(); // append filename to path

    fp = fopen(path_to_file.data(), "wb"); // open file
    if(fp == nullptr) error_call(FILE_IO_ERROR, "Error occured when opening file");

    while(remainingData) {
        if(remainingData < BUFFER_SIZE) {
            len = recv(m_socket, buffer, remainingData, 0); // read from socket
            fwrite(buffer, sizeof(char), len, fp); // write to file
            remainingData -= len;
            //printf("Received %lu bytes, expecting %d bytes\n", len, remainingData);
            memset(buffer, 0, BUFFER_SIZE); // reset bufer
            break;
        } else {
            len = recv(m_socket, buffer, BUFFER_SIZE, 0); //256
            fwrite(buffer, sizeof(char), len, fp);
            remainingData -= len;
            //printf("Received %lu bytes, expecting: %d bytes\n", len, remainingData);
        }
        memset(buffer, 0, BUFFER_SIZE); // reset buffer
    }
    fclose(fp);
}

void sftpClient::send_file(std::string filename, int sockfd, int filesize) const {
    FILE *fp; // file hadnle
    int n;
    int read_bytes;
    int bytes_left = filesize;


    fp = fopen(filename.data(), "rb"); // open file
    if(fp == nullptr) error_call(FILE_IO_ERROR, "Error occured while opening " + filename);
    PRINT2("cl: file opened", bytes_left);
    char data[BUFFER_SIZE];
    memset(data, 0, BUFFER_SIZE);


    while (bytes_left) {
        if(bytes_left < BUFFER_SIZE) {
            read_bytes = fread(data, 1, bytes_left, fp);
            bytes_left = bytes_left - read_bytes;
            n = send(m_socket, data, bytes_left + read_bytes, 0);
            if(n < 0) printf("Error sending slab\n");
            printf("sent %d slab\n", read_bytes);
        } else {
            read_bytes = fread(data, 1, BUFFER_SIZE, fp);
            bytes_left -= read_bytes;
            n = send(m_socket, data, BUFFER_SIZE, 0);
            if(n < 0) printf("Error sending slab\n");
            printf("sent %d slab\n", read_bytes);
        }
    }
    printf("File sent!\n");
    fclose(fp);
}

void sftpClient::parse_user_input(const std::string& user_input) {
    tokenize(user_input, ' ', m_tquery);
    auto cmd = m_tquery.front(); // save command
    make_str_upper(cmd); // make command uppercase

    if(cmd == "RETR") {
        if(m_tquery.size() > 1) // correct count of tokens
        m_retr_filename = fs::path(m_tquery[1].data()); // save filename
    }
    if(cmd == "DONE") {
        m_done = true;
    }
    if(cmd == "STOR") {
        if(m_tquery.size() == 3 && m_logged_in) {
            m_send_filename = m_tquery[2]; // save filename
            m_stor_planned = true; // set state var
        }
    }
    if(cmd == "SIZE") {
    }
    if(cmd == "USER") {
        if(m_tquery.size() == 2) {
            m_username = m_tquery[1]; // save username
        }
    }


}
