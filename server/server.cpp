//
// Created by Andrej Hyros on 02/04/2022.
//
#include "server.h"
#include "../utils/error.h"
#include "../utils/hints.h"

#include <net/if.h>
#include <sys/ioctl.h>

sftpServer::sftpServer(ArgParserServer *args) {
    gethostname(m_hostname, MAX_HOSTNAME_LEN);
    memset(&m_hints, 0, sizeof(m_hints));
    memset(&m_buffer, 0, BUFFER_SIZE);
    m_hints.ai_family = AF_UNSPEC;
    m_hints.ai_socktype = SOCK_STREAM;
    m_hints.ai_flags = AI_PASSIVE;
    m_wdir = fs::current_path();
    m_stream_type = BINARY;
    m_args = args;
    m_port = args->m_pArg;
}

void *sftpServer::get_in_addr(sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((sockaddr_in *)sa)->sin_addr);
    }
    return &(((sockaddr_in6 *)sa)->sin6_addr);
}

int sftpServer::bind_to(addrinfo *ptr, int& yes, addrinfo *servinfo) {
    int sock = 0;


    for(ptr = servinfo; ptr != nullptr; ptr = ptr->ai_next) {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if(sock == -1) {
            PRINT("SOCKET ERROR");
            continue;
        }

        if(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(int)) == -1) {  // make socket reusable
            PRINT2("SETSOCKOPT ERROR", errno);
            exit(1);
        }

        if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) { // make socket reusable
            PRINT("SETSOCKOPT ERROR");
            exit(1);
        }



        auto *test = reinterpret_cast<sockaddr_in*>(ptr);
        char *ip = inet_ntoa(test->sin_addr);
        //PRINT(ip);

        if(bind(sock, ptr->ai_addr, ptr->ai_addrlen) == -1) { // bind
            close(sock);
            PRINT2("BIND ERROR: ", errno);
            continue;
        }
        break;
    }
    return sock;
}

void sftpServer::start() {
    //PRINT("[SERVER] Starting...");
    int yes = 1;
    addrinfo *servinfo, *p; // pointers to addrinfo

    if(getaddrinfo(nullptr, m_port.data(), &m_hints, &servinfo) != 0) {
        error_call(CONNECTION_ERROR, "gai_err", 1);
    }

    //PRINT("[SERVER] Waiting for connections...");
    m_sock = bind_to(p, yes, servinfo);
    freeaddrinfo(servinfo);
    if(!p) {
        error_call(CONNECTION_ERROR, "server:failed to bind", errno);
    }
    if(listen(m_sock, BACKLOG) == -1) { // listen
        error_call(CONNECTION_ERROR, "server: listen", errno);
    }

    accept_connection();
}

void sftpServer::accept_connection() {
    socklen_t sin_size = sizeof(sockaddr_storage); // for accept
    m_socket = accept(m_sock, (sockaddr*)&m_client_addr, &sin_size); // accept connection
    if(m_socket == -1) {
        error_call(CONNECTION_ERROR, "server: accept", errno);
    }

    inet_ntop(m_client_addr.ss_family, get_in_addr((sockaddr*)&m_client_addr), m_ipaddr, sizeof(m_ipaddr));
    //PRINT2("server: got connection from", m_ipaddr);

    start_conversation();
}

void sftpServer::close_connection() const {
    shutdown(m_socket, SHUT_RDWR);
    //close(m_socket);
}

void sftpServer::start_conversation() {
    int numbytes; // number of received bytes
    std::string greeting("+" + std::string(m_hostname) + " SFTP SERVICE"); // greetin message
    PRINT(greeting);
    send(m_socket, greeting.data(), greeting.length(), 0); // send greeting message

    while(true) { // communication loop
        numbytes = receive(); // receive answer
        if(!numbytes) { // connection closed by remote peer
            PRINT("Connection closed by remote peer");
            break;
        }
        PRINT(m_buffer); // print clients query

        parse_query(); // parse clients quer

        send(m_socket, m_buffer, strlen(m_buffer), 0); // send answer
        PRINT(m_buffer); // print answer

        if(m_done) break;
    }
    close_connection();
    reset_server_settings();
    accept_connection();
}

void sftpServer::parse_query() {
    std::string query(m_buffer);
    memset(m_buffer, 0, BUFFER_SIZE); // clear buffer for reply
    tokenize(query, ' ', m_tquery); // tokenize the query by space
    make_str_upper(m_tquery.front()); // make command uppercase
    check_tobe();

    switch (code_string(m_tquery.front())) {
        case USER:
        case ACCT:
            cmd_user();
            break;
        case PASS:
            cmd_pass();
            break;
        case TYPE:
            cmd_type();
            break;
        case LIST:
            cmd_list();
            break;
        case CDIR:
            cmd_cdir();
            break;
        case KILL:
            cmd_kill();
            break;
        case NAME:
            cmd_name();
            break;
        case DONE:
            cmd_done();
            break;
        case RETR:
            cmd_retr();
            break;
        case STOR:
            cmd_stor();
            break;
        case TOBE:
            cmd_tobe();
            break;
        case STOP:
            cmd_stop();
            break;
        case SEND:
            cmd_send();
            break;
        case SIZE:
            cmd_size();
            break;
        case ERROR:
        default:
            load_buffer("-Invalid query");
            break;
    }
    m_tquery.clear();
}


// =====================================================================================================================


void sftpServer::cmd_user() {
    if(!is_valid_count(2)) return;
    if(m_logged_in) { // user is logged in
        load_buffer("-Already logged in");
        return;
    }
    auto user_id = m_tquery[1];


    if(m_tquery[1] == m_userid) { // password is correct and userid was provided
        load_buffer("!" + m_userid + " logged in");
        m_logged_in = true;
        return;
    }
    if(is_valid_user(user_id, 0)) { // user is valid
        load_buffer("+" + m_userid + " valid, send password");
        m_userid_sent = true;
        return;
    }
    load_buffer("-Invalid user-id, try again"); // self-explanatory
}

void sftpServer::cmd_pass() {
    if(!is_valid_count(2)) return;
    if(m_logged_in) { // user is logged in
        load_buffer("-Already logged in");
        return;
    }

    if(m_tquery[1] == m_password) { // password is correct and userid was provided
        load_buffer("!" + m_userid + " logged in");
        m_logged_in = true;
        return;
    }

    if(is_valid_user(m_tquery[1], 1)) { // user is valid
        load_buffer("+" + m_tquery[1] + " valid, send userid");
        m_password_sent = true;
        return;
    }

    load_buffer("-Wrong password, try again");
}

void sftpServer::cmd_type() {
    if(!is_valid_count(2)) return;
    if(!is_logged_in()) return; // user is not logged in
    if(m_tquery[1] == "A") {
        m_stream_type = ASCII;
        load_buffer("+Using Ascii mode");
        return;
    }
    if(m_tquery[1] == "B") {
        m_stream_type = BINARY;
        load_buffer("+Using Binary mode");
        return;
    }
    if(m_tquery[1] == "C") {
        m_stream_type = CONTINUOUS;
        load_buffer("+Using Continuous mode");
        return;
    }
    load_buffer("-Type not valid");
}

void sftpServer::cmd_list() {
    bool verbose = false; // verbose set to false by default
    fs::path path = m_wdir;
    std::string reply;

    if(!is_valid_count(2, 3)) return; // bad arg count
    if(!is_logged_in()) return; // user is not logged in

    if(m_tquery[1] == "V") verbose = true;
    if(!verbose && m_tquery[1] != "F") { // invalid arg
        load_buffer(query_hints[m_tquery.front()]);
        return;
    }
    if(m_tquery.size() == 3) { // path was provided
        path = m_tquery[2];
        if(!fs::exists(path)) { // folder doesnt exist
            load_buffer("-Folder does not exist, try again...");
            return;
        }
        if(path.string() == "..") {
            path = m_wdir / path;
            get_rid_of_parents(path);
        }
    }
    reply = "+" + path.string() + ":\n"; // assemble first line of response

    for(auto &item : fs::directory_iterator(path)) { // for verbose answer
        if(verbose) {
            uint filesize = !fs::is_directory(item.path()) ? fs::file_size(item.path()) : 0; // show filesizes only for files
            reply += item.path().filename().string() + " | " + (!fs::is_directory(item.path()) ? std::to_string(filesize) + "B " : "" )+ "\n"; // assebmle response
            continue;
        }
        reply += item.path().filename().string() + "\n";
    }
    load_buffer(reply);
}

void sftpServer::cmd_cdir() {
    if(!is_valid_count(2)) return;
    if(!is_logged_in()) return; // user is not logged in
    fs::path path(m_tquery[1]);

    if(path.is_relative()) { // user passed relative path
        if(path.string() == "..") { // user passed ".."
            path = m_wdir.parent_path();
        } else {
            path = m_wdir / path;
        }
    }
    get_rid_of_parents(path);
    PRINT(path.string());
    if(!fs::exists(path)) { // file does not exist
        load_buffer("-Can't connect to directory because: does not exist");
        return;
    }
    if(!fs::is_directory(path)) { // path leads to folder
        load_buffer("-Can't connect to directory because: is not directory");
        return;
    }
    m_wdir = fs::absolute(path); // update current working dir
    load_buffer("+Changed directory to " + m_wdir.string());
}

void sftpServer::cmd_kill() {
    if(!is_valid_count(2)) return;
    if(!is_logged_in()) return; // user is not logged in
    fs::path file(m_tquery[1]);

    std::error_code ec;
    if(fs::remove(file, ec)) { // deletion succesful
        load_buffer("+" + file.string() + " deleted");
        return;
    } else {
        load_buffer("-Not deleted - " + ec.message());
    }
}

void sftpServer::cmd_name() {
    if(!is_valid_count(2)) return;
    if(!is_logged_in()) return; // user is not logged in
    fs::path path(m_tquery[1]);
    if(!fs::exists(path)) { // file does not exist
        load_buffer("-Can't find " + path.string());
        return;
    }
    m_path_to_be_renamed = path;
    m_NAME = true; // set state variable
    load_buffer("+File exists");
}

void sftpServer::cmd_tobe() {
    if(!is_valid_count(2)) return;
    if(!m_NAME) { // NAME wasnt last query
        load_buffer("-Send NAME query first");
        return;
    }
    fs::path new_name(m_tquery[1]);

    fs::path path_to_file = m_path_to_be_renamed.parent_path();
    if(fs::exists(new_name)) { // file exists
        load_buffer("-Failed to rename file, reason: name already taken");
        return;
    }
    try { // try renaming
        fs::rename(m_path_to_be_renamed, path_to_file/new_name);
    }
    catch (...) {
        load_buffer("-Failed to rename file, reason: unknown");
        return;
    }
    load_buffer( "+" + m_path_to_be_renamed.string() + " renamed to " + new_name.string());
}

void sftpServer::cmd_done() {
    load_buffer("+OK, bye!");
    m_done = true;
}

void sftpServer::cmd_retr() {
    if(!is_valid_count(2)) return;
    if(!is_logged_in()) return; // user is not logged in
    fs::path file(m_tquery[1]);
    if(!fs::exists(file)) {
        load_buffer("-File does not exist");
        return;
    }
    size_t size = fs::file_size(file); // save filesize
    m_retr_planned = true; // set state var
    m_retrieved_filename = file.string(); // save filename
    m_retrieved_filesize = size; // set size
    load_buffer(std::to_string(size));
}

void sftpServer::cmd_stop() {
    if(!m_retr_planned) {
        load_buffer("-No RETR query to be stopped");
        return;
    }
    m_retr_planned = false;
    load_buffer("+ok, RETR aborted");
}

void sftpServer::cmd_send() {
    if(m_retr_planned) {
        send_file(m_retrieved_filename);
        load_buffer("+File send successfully");
        //m_retr_planned = false;
        return;
    }
    load_buffer("-Send RETR query first");
}

void sftpServer::cmd_stor() {
    if(!is_valid_count(3)) return;
    if(!is_logged_in()) return; // user is not logged in

    auto type = m_tquery[1];
    auto filename = fs::path(m_tquery[2]).filename().string(); // store only filename, not whole path
    bool exists = fs::exists(fs::path(filename)); // file existis
    m_stored_filename = filename; // save filenmae
    m_stor_planned = true; // set state var

    if(!(type == "NEW" || type == "OLD" || type == "APP")) { // not a valid option
        load_buffer(query_hints[m_tquery.front()]);
    }
    if (type == "NEW") {
        if(exists) {
            m_stored_filename = regex_replace(m_stored_filename, std::regex("\\."), "-copy.");
            load_buffer("+File exists, will create file with name " + m_stored_filename);
            return;
        }
        load_buffer("+File does not exist, will create new file");
    }
    if (type == "OLD") {
        if(exists) {
            load_buffer("+Will write over old file");
            return;
        }
        load_buffer("-File does not exist");
    }
    if(type == "APP") {
        load_buffer("-Appending not supported");
        m_stor_planned = false;
    }
}

void sftpServer::cmd_size() {
    if(!is_valid_count(2)) return;
    if(!m_stor_planned) { // stor is not planned
        load_buffer("-Send STOR query first");
        return;
    }
    if(!is_number(m_tquery[1])) { // filesize arg not valid
        load_buffer("-Not a valid filesize");
        return;
    }
    m_stored_filesize = atoi(m_tquery[1].data());
    retrieve_file();
    load_buffer("+Saved " + m_stored_filename);
    m_stor_planned = false; // set state var
}


// =====================================================================================================================


void sftpServer::retrieve_file() {
    char buffer[BUFFER_SIZE]; // buffer for data
    bzero(buffer, BUFFER_SIZE); // set it to zeros
    int bytes_left = m_stored_filesize;
    ssize_t len;
    FILE *fp; // file handle

    std::string path_to_storage = m_args->m_fArg + "/" + m_stored_filename; // add slash and filename to path
    //fp = fopen(m_stored_filename.data(), "wb");
    fp = fopen(path_to_storage.data(), "wb");
    if(fp == nullptr) { // opening file was unsuccesful
        error_call(FILE_IO_ERROR, "Error with opening file");
    }

    while(bytes_left) {
        if(bytes_left < BUFFER_SIZE) { // left bytes can be fit into single buffer
            len = recv(m_socket, buffer, bytes_left, 0);
            fwrite(buffer, sizeof(char), len, fp);
            bytes_left -= len;
            //printf("Received %lu bytes, expecting %d bytes\n", len, bytes_left);
            memset(buffer, 0, BUFFER_SIZE);
            break;
        } else { // there are more then BUFFER_SIZE bytes left
            len = recv(m_socket, buffer, BUFFER_SIZE, 0); //256
            fwrite(buffer, sizeof(char), len, fp);
            bytes_left -= len;
            //printf("Received %lu bytes, expecting: %d bytes\n", len, bytes_left);
        }
        memset(buffer, 0, BUFFER_SIZE); // reset buffer
    }
    fclose(fp); // close file
}

void sftpServer::send_file(std::string filename) {
    FILE *fp; // file handle
    ssize_t n; // number of bytes sent
    size_t read_bytes; // number of bytes read from file
    int bytes_left = m_retrieved_filesize;

    fp = fopen(filename.data(), "rb");
    if(fp == nullptr) error_call(FILE_IO_ERROR, "Error occured while opening " + filename);
    char data[BUFFER_SIZE];
    memset(data, 0, BUFFER_SIZE);


    while (bytes_left) {
        if(bytes_left < BUFFER_SIZE) {
            read_bytes = fread(data, 1, bytes_left, fp);
            bytes_left = bytes_left - read_bytes;
            n = send(m_socket, data, bytes_left + read_bytes, 0);
            if(n < 0) printf("Error sending slab\n");
            //printf("sent %d slab\n", read_bytes);
        } else {
            read_bytes = fread(data, 1, BUFFER_SIZE, fp);
            bytes_left -= read_bytes;
            n = send(m_socket, data, BUFFER_SIZE, 0);
            if(n < 0) printf("Error sending slab\n");
            //printf("sent %d slab\n", read_bytes);
        }
    }
    fclose(fp); // close file
}

void sftpServer::check_tobe() {
    if(m_NAME) { // NAME was last query
        if(m_tquery.front() != "TOBE") m_NAME = false; // new query is not TOBE
    }
    if(m_stor_planned) { // stor action is planned
        if(m_tquery.front() != "SIZE") m_stor_planned = false; // new query is not SIZE
    }
}

int sftpServer::receive() {
    int numbytes; // number of bytes read
    memset(m_buffer, 0, BUFFER_SIZE); // clear buffer for query
    if((numbytes = recv(m_socket, m_buffer, BUFFER_SIZE-1, 0)) == -1) { // receive query
        error_call(TRANSMISSION_ERROR, "recv error, exiting...");
    }
    return numbytes;
}

bool sftpServer::is_valid_count(int cnt, int upto) {
    int len = m_tquery.size(); // number of tokens
    if(upto) { // upper boundry was set
        if(len < cnt || len > upto) { // count is not in interval
            load_buffer(query_hints[m_tquery.front()]);
            return false;
        }
        return true;
    }
    if(len != cnt) { // not valid count of tokens
        load_buffer(query_hints[m_tquery.front()]);
        return false;
    }
    return true;
}

void sftpServer::load_buffer(std::string msg) {
    memcpy(m_buffer, msg.data(), msg.size());
}

bool sftpServer::is_valid_user(std::string token, bool is_password) {
    std::vector <std::string> userpasses; // holds all lines from userpass.txt
    std::vector <std::string> pair; // will hold user and pass of single line
    bool ok = load_file(userpasses, m_args->m_uArg.data()); // load file
    if (!ok) error_call(FILE_IO_ERROR, "Loading userpass.txt failed"); // error
    for (auto userpass: userpasses) { // iterate and find match
        tokenize(userpass, ':', pair);
        if (token == pair[is_password]) { // match
            m_userid = pair.front(); // user is valid, save userid and password to object
            m_password = pair.back();
            return true;
        }
        pair.clear();
    }
    return false;
}

bool sftpServer::is_logged_in() {
    if(!m_logged_in) {
        load_buffer("-You must log in first");
        return false;
    }
    return true;
}

void sftpServer::reset_server_settings() {
    m_logged_in = false;
    m_userid_sent = false;
    m_password_sent = false;
    m_NAME = false;
    m_done = false;
    m_retr_planned = false;
    m_stor_planned = false;
    m_userid.clear();
    m_password.clear();
    m_retrieved_filename.clear();
    m_retrieved_filesize = 0;
    m_stored_filename.clear();
    m_stored_filesize = 0;
    m_wdir = fs::current_path();
    m_path_to_be_renamed.clear();
    m_stream_type = BINARY;
}