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

#include <arpa/inet.h>
#define PRINT(x) std::cout << x << std::endl
#define PORT "5060"
#define LOCALHOST "127.0.0.1"
#define MAX_DATA_SIZE 500

// CMD + SHIFT + ENTER  == ; na koniec line

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}


int main(int argc, char *argv[]) {
    int sock, numbytes;
    char buffer[MAX_DATA_SIZE];
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


    if((numbytes = recv(sock, buffer, MAX_DATA_SIZE-1, 0)) == -1) {
        PRINT("recv error, exiting...");
        exit(0);
    }
    PRINT(buffer);
    while(true) {

        buffer[numbytes] = '\0';
        memset(buffer, 0, MAX_DATA_SIZE);
        std::getline(std::cin, user_input);
        if(user_input == "!close") {
            break;
        }
        memcpy(buffer, user_input.data(), user_input.length());
        send(sock, buffer, user_input.length(), 0);
        memset(buffer, 0, MAX_DATA_SIZE);

        // read from sercer
        if((numbytes = recv(sock, buffer, MAX_DATA_SIZE-1, 0)) == -1) {
            PRINT("recv error, exiting...");
            exit(0);
        }
        // print server msg
        PRINT(buffer);
        memset(buffer, 0, MAX_DATA_SIZE);


    }


    close(sock);



    return 0;
}
