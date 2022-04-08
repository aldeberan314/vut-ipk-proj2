#include <iostream>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <vector>
#include <fstream>

void error_call(int errcode, std::string errmsg) {
    std::cerr << errmsg << " " <<  "\n";
    exit(errcode);
}


#define PRINT(x) std::cout << x << std::endl
#define PRINT2(x, y) std::cout << x << " " << y <<  std::endl
#define PORT "5060"
#define LOCALHOST "127.0.0.1"
#define BUFFER_SIZE 1024

// CMD + SHIFT + ENTER  == ; na koniec line

typedef unsigned char BYTE;

std::string savename;




void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

void send_file(std::string filename, int sockfd, int filesize) {
    PRINT("in send_file");
    FILE *fp;
    int n;
    int read_bytes;
    int bytes_left = filesize;


    fp = fopen(filename.data(), "rb");
    if(fp == nullptr) error_call(3, "Error occured while opening " + filename);
    PRINT2("cl: file opened", bytes_left);
    char data[BUFFER_SIZE];
    memset(data, 0, BUFFER_SIZE);


    while (bytes_left) {
        if(bytes_left < BUFFER_SIZE) {
            read_bytes = fread(data, 1, bytes_left, fp);
            bytes_left = bytes_left - read_bytes;
            n = send(sockfd, data, bytes_left + read_bytes, 0);
            if(n < 0) printf("Error sending slab\n");
            printf("sent %d slab\n", read_bytes);
        } else {
            read_bytes = fread(data, 1, BUFFER_SIZE, fp);
            bytes_left -= read_bytes;
            n = send(sockfd, data, BUFFER_SIZE, 0);
            if(n < 0) printf("Error sending slab\n");
            printf("sent %d slab\n", read_bytes);
        }
    }
    printf("File sent!\n");
    fclose(fp);
    return;   
}


void retrieve_file(int sockfd, size_t filesize) {

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    int remainingData = filesize;
    ssize_t len;
    FILE *fp;


    fp = fopen(savename.data(), "wb");
    if(fp == nullptr) {
        PRINT("Error with opening file");
        exit(0);
    }

    while(remainingData) {
        if(remainingData < BUFFER_SIZE) {
            len = recv(sockfd, buffer, remainingData, 0);
            fwrite(buffer, sizeof(char), len, fp);
            remainingData -= len;
            printf("Received %lu bytes, expecting %d bytes\n", len, remainingData);
            memset(buffer, 0, BUFFER_SIZE);
            break;
        } else {
            len = recv(sockfd, buffer, BUFFER_SIZE, 0); //256
            fwrite(buffer, sizeof(char), len, fp);
            remainingData -= len;
            printf("Received %lu bytes, expecting: %d bytes\n", len, remainingData);
        }
        memset(buffer, 0, BUFFER_SIZE);
    }
    fclose(fp);
    return;
}


int main(int argc, char *argv[]) {
    savename = argv[1];
    int sock, numbytes, filesize;
    char buffer[BUFFER_SIZE];
    addrinfo hints, *servinfo, *p;
    int gai_r;
    char s[INET6_ADDRSTRLEN];
    std::string user_input;

    memset(&hints, 0, sizeof(hints));
    hints.ai_addr = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    gai_r = getaddrinfo(LOCALHOST, PORT, &hints, &servinfo);
    if(gai_r != 0) {
        PRINT("gai error, exiting");
        exit(1);
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(sock == -1) {
            PRINT("socket error, continuing...");
            //exit(0);
            continue;
        }
        if(connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            PRINT("connect error, continueing...");
            continue;
        }
        break;
    }
    if (p == NULL) {
        PRINT("client: failed to connect, returning");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((sockaddr*)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);


    if((numbytes = recv(sock, buffer, BUFFER_SIZE-1, 0)) == -1) {
        PRINT("recv error, exiting...");
        exit(0);
    }
    PRINT(buffer);
    while(true) {

        buffer[numbytes] = '\0';
        memset(buffer, 0, BUFFER_SIZE);
        std::getline(std::cin, user_input);
        if(user_input == "DONE") {
            break;
        }
        memcpy(buffer, user_input.data(), user_input.length());
        send(sock, buffer, user_input.length(), 0);
        memset(buffer, 0, BUFFER_SIZE);


        if(user_input[0] == 'S' && user_input[1] == 'I' && user_input[2] == 'Z' && user_input[3] == 'E') {
            PRINT(user_input.data() + 4);
            send_file(argv[1], sock, atoi(user_input.data() + 4));
        }


        if(user_input == "SEND") {
            retrieve_file(sock, filesize);
        }

        // read from server
        if((numbytes = recv(sock, buffer, BUFFER_SIZE-1, 0)) == -1) {
            PRINT("recv error, exiting...");
            exit(0);
        }
        //PRINT("POST RECV");

        // todo tato podmienka plz
        if(user_input[0] == 'R' && user_input[1] == 'E' && user_input[2] == 'T' && user_input[3] == 'R') {
            filesize = atoi(buffer);
        }

        // print server msg
        PRINT(buffer);
        memset(buffer, 0, BUFFER_SIZE);


    }

    close(sock);



    return 0;
}
