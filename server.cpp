//
// Created by Andrej Hyros on 02/04/2022.
//
#include "server.h"
#include "error.h"




sftpServer::sftpServer() {
    gethostname(m_hostname, MAX_HOSTNAME_LEN);
    memset(&m_hints, 0, sizeof(m_hints));
    memset(&m_buffer, 0, BUFFER_SIZE);
    m_hints.ai_family = AF_UNSPEC;
    m_hints.ai_socktype = SOCK_STREAM;
    m_hints.ai_flags = AI_PASSIVE;
}

void *sftpServer::get_in_addr(sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((sockaddr_in *)sa)->sin_addr);
    }
    return &(((sockaddr_in6 *)sa)->sin6_addr);
}

int sftpServer::bind_to(addrinfo *ptr, int& yes, addrinfo *servinfo) {
    int sock = 0;
    for(ptr = servinfo; ptr != NULL; ptr = ptr->ai_next) {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if(sock == -1) {
            PRINT("SOCKET ERROR");
            continue;
        }
        if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            PRINT("SETSOCKOPT ERROR");
            exit(1);
        }
        if(bind(sock, ptr->ai_addr, ptr->ai_addrlen) == -1) {
            close(sock);
            PRINT("BIND ERROR");
            continue;
        }
        break;
    }
    return sock;
}

void sftpServer::start() {
    PRINT("[SERVER] Starting...");
    int sock; // socket file descriptors
    int yes = 1;
    int numbytes;
    socklen_t sin_size = sizeof(sockaddr_storage); // for accept
    addrinfo *servinfo, *p; // pointers to addrinfo

    if(getaddrinfo(NULL, PORT, &m_hints, &servinfo) != 0) {
        error_call(1, "gai_err", 1);
    }

    PRINT("[SERVER] Waiting for connections...");
    sock = bind_to(p, yes, servinfo);
    freeaddrinfo(servinfo);
    if(!p) {
        error_call(1, "server:failed to bind", errno);
    }

    if(listen(sock, BACKLOG) == -1) {
        error_call(1, "server: listen", errno);
    }

    m_socket = accept(sock, (sockaddr*)&m_client_addr, &sin_size);
    if(m_socket == -1) {
        error_call(1, "server: accept");
    }

    inet_ntop(m_client_addr.ss_family, get_in_addr((sockaddr*)&m_client_addr), m_ipaddr, sizeof(m_ipaddr));
    PRINT2("server: got connection from", m_ipaddr);

    start_conversation();
}

void sftpServer::closeConnection() {
    close(m_socket);
}

void sftpServer::start_conversation() {
    int numbytes;
    std::string greeting("+" + std::string(m_hostname) + " SFTP SERVICE");
    send(m_socket, greeting.data(), greeting.length(), 0); // send greeting message

    while(true) {
        //recv
        memset(m_buffer, 0, BUFFER_SIZE);
        if((numbytes = recv(m_socket, m_buffer, BUFFER_SIZE-1, 0)) == -1) {
            PRINT("recv error, exiting...");
            exit(0);
        }
        if(!numbytes) { // connection closed by remote peer
            break;
        }
        //parse query
        parse_query();

        //respond
        send(m_socket, "[SERVER] answer from server...", 30, 0);
    }

    closeConnection();
}

void sftpServer::parse_query() {
    std::string query(m_buffer);
    const char delimiter = ' ';
    tokenize(query, delimiter, m_tquery);

    //USER ! ACCT ! PASS ! TYPE ! LIST ! CDIR ! KILL ! NAME ! DONE ! RETR ! STOR

    switch (hash_string(m_tquery.front())) {
        case USER:
            PRINT("USER query");
            break;
        case ACCT:
            break;
        case PASS:
            break;
        case TYPE:
            break;
        case LIST:
            break;
        case CDIR:
            break;
        case KILL:
            break;
        case NAME:
            break;
        case DONE:
            break;
        case RETR:
            break;
        case STOR:
            break;
        default:
            PRINT("[SERVER] Unknown query");
            break;
    }


    m_tquery.clear();
    //for(auto token : m_tquery) PRINT(token);

}

void sftpServer::tokenize(std::string const &str, const char delim,
              std::vector<std::string> &out)
{
    size_t start;
    size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }
}
