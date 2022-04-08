//
// Created by Andrej Hyros on 08/04/2022.
//

#include "client.h"


sftpClient::sftpClient() {
    memset(&m_hints, 0, sizeof(m_hints));
    memset(m_buffer, 0, BUFFER_SIZE);
    m_hints.ai_addr = AF_UNSPEC;
    m_hints.ai_socktype = SOCK_STREAM;
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


    gai_r = getaddrinfo(LOCALHOST, PORT, &m_hints, &servinfo);
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
    printf("client: connecting to %s\n", m_ipaddr);

    freeaddrinfo(servinfo);

    start_conversation();

    close(m_socket);
}

void sftpClient::start_conversation() {
    int rec_bytes, filesize;
    std::string user_input;

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

        if(user_input == "DONE") {
            break;
        }
        memcpy(m_buffer, user_input.data(), user_input.length()); // load input to buffer
        send(m_socket, m_buffer, user_input.length(), 0); // send it
        memset(m_buffer, 0, BUFFER_SIZE); // prepare buffer for read


        if(user_input[0] == 'S' && user_input[1] == 'I' && user_input[2] == 'Z' && user_input[3] == 'E') {
            PRINT(user_input.data() + 4);
            send_file("random_file.txt", m_socket, atoi(user_input.data() + 4));
        }


        if(user_input == "SEND") {
            retrieve_file(m_socket, filesize);
        }

        if((rec_bytes = recv(m_socket, m_buffer, BUFFER_SIZE-1, 0)) == -1) { // read from server
            PRINT("recv error, exiting...");
            exit(0);
        }

        // todo tato podmienka plz
        if(user_input[0] == 'R' && user_input[1] == 'E' && user_input[2] == 'T' && user_input[3] == 'R') {
            filesize = atoi(m_buffer);
        }

        // print server msg
        PRINT(m_buffer);
        memset(m_buffer, 0, BUFFER_SIZE);


    }
}

void sftpClient::retrieve_file(int sockfd, size_t filesize) {

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    int remainingData = filesize;
    ssize_t len;
    FILE *fp;


    fp = fopen(m_retr_filename.filename().string().data(), "wb");
    PRINT(m_retr_filename.string());
    PRINT(m_retr_filename.filename().string().data());
    if(fp == nullptr) error_call(FILE_IO_ERROR, "Error occured when opening file");

    while(remainingData) {
        if(remainingData < BUFFER_SIZE) {
            len = recv(m_socket, buffer, remainingData, 0);
            fwrite(buffer, sizeof(char), len, fp);
            remainingData -= len;
            printf("Received %lu bytes, expecting %d bytes\n", len, remainingData);
            memset(buffer, 0, BUFFER_SIZE);
            break;
        } else {
            len = recv(m_socket, buffer, BUFFER_SIZE, 0); //256
            fwrite(buffer, sizeof(char), len, fp);
            remainingData -= len;
            printf("Received %lu bytes, expecting: %d bytes\n", len, remainingData);
        }
        memset(buffer, 0, BUFFER_SIZE);
    }
    fclose(fp);
    return;
}

void sftpClient::send_file(std::string filename, int sockfd, int filesize) {
    PRINT("in send_file");
    FILE *fp;
    int n;
    int read_bytes;
    int bytes_left = filesize;


    fp = fopen(filename.data(), "rb");
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
    return;
}

void sftpClient::parse_user_input(std::string user_input) {
    std::vector<std::string> tinput;
    tokenize(user_input, ' ', tinput);
    auto cmd = tinput.front();


    if(cmd == "RETR") {
        if(tinput.size() != 2) exit(0);
        m_retr_filename = fs::path(tinput[1].data());
    }

}
