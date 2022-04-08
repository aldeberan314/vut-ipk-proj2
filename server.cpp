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
    m_wdir = fs::current_path();
    m_stream_type = BINARY;
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

        if(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(int)) == -1) {
            PRINT2("SETSOCKOPT ERROR", errno);
            exit(1);
        }

        if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            PRINT("SETSOCKOPT ERROR");
            exit(1);
        }

        if(bind(sock, ptr->ai_addr, ptr->ai_addrlen) == -1) {
            close(sock);
            PRINT2("BIND ERROR: ", errno);
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
    socklen_t sin_size = sizeof(sockaddr_storage); // for accept
    addrinfo *servinfo, *p; // pointers to addrinfo

    if(getaddrinfo(NULL, PORT, &m_hints, &servinfo) != 0) {
        error_call(CONNECTION_ERROR, "gai_err", 1);
    }


    PRINT("[SERVER] Waiting for connections...");
    sock = bind_to(p, yes, servinfo);
    m_sonk = sock;
    freeaddrinfo(servinfo);
    if(!p) {
        error_call(CONNECTION_ERROR, "server:failed to bind", errno);
    }
    if(listen(sock, BACKLOG) == -1) {
        error_call(CONNECTION_ERROR, "server: listen", errno);
    }

    accept_connection(sock);
}

void sftpServer::accept_connection(int sonk) {
    socklen_t sin_size = sizeof(sockaddr_storage); // for accept
    m_socket = accept(sonk, (sockaddr*)&m_client_addr, &sin_size);
    if(m_socket == -1) {
        error_call(CONNECTION_ERROR, "server: accept", errno);
    }

    inet_ntop(m_client_addr.ss_family, get_in_addr((sockaddr*)&m_client_addr), m_ipaddr, sizeof(m_ipaddr));
    PRINT2("server: got connection from", m_ipaddr);

    start_conversation();
}

void sftpServer::close_connection() {
    shutdown(m_socket, SHUT_RDWR);
    //close(m_socket);
}

void sftpServer::start_conversation() {
    int numbytes;
    std::string greeting("+" + std::string(m_hostname) + " SFTP SERVICE");
    PRINT(greeting);
    send(m_socket, greeting.data(), greeting.length(), 0); // send greeting message

    while(true) {
        //recv
        numbytes = receive();
        if(!numbytes) { // connection closed by remote peer
            PRINT("Connection closed by remote peer");
            break;
        }
        //parse query
        parse_query();

        //respond
        send(m_socket, m_buffer, strlen(m_buffer), 0);
        PRINT(m_buffer);

        if(m_done) break;
    }
    close_connection();
    reset_server_settings();
    this->accept_connection(m_sonk);
}

void sftpServer::parse_query() {
    std::string query(m_buffer);
    memset(m_buffer, 0, BUFFER_SIZE); // clear buffer for reply
    tokenize(query, ' ', m_tquery);
    check_tobe();

    switch (code_string(m_tquery.front())) {
        case USER:
            cmd_user();
            break;
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
    if(!is_valid_count(2,"-Invalid query, usage: \"USER <userid>\"")) return;
    if(m_logged_in) {
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

// TODO pri prikazoch kontrolovat ci je user logged in, nejaku fciu si na to napisat
void sftpServer::cmd_pass() {
    if(!is_valid_count(2,"-Invalid query, usage: \"PASS <password>\"")) return;
    if(m_logged_in) {
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

//todo logged in
void sftpServer::cmd_type() {
    if(!is_valid_count(2, "-Invalid query, usage: \"TYPE { A | B | C }\"")) return;
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

// TODO clean up a bit
void sftpServer::cmd_list() {
    bool verbose = false;
    fs::path path = m_wdir;
    std::string reply;

    if(!is_valid_count(2,"-Invalid query, usage: \"LIST { F | V } directory-path\"", 3)) return;

    if(m_tquery[1] == "V") verbose = true;
    if(!verbose && m_tquery[1] != "F") { // invalid arg
        load_buffer("-Invalid query, usage: \"LIST { F | V } directory-path\"");
        return;
    }
    if(m_tquery.size() == 3) { // path was provided
        path = m_tquery[2];
        if(!fs::exists(path)) { // folder doesnt exist
            load_buffer("-Folder does not exist, try again...");
            return;
        }
    }
    reply = "+" + path.string() + ":\n";
    if(verbose) {
        // TODO add more information for verbose option (only filesize may not be sufficient)
        for(auto &item : fs::directory_iterator(path)) { // for verbose answer
            uint filesize = !fs::is_directory(item.path()) ? fs::file_size(item.path()) : 0;
            reply += item.path().filename().string() + " | " + (!fs::is_directory(item.path()) ? std::to_string(filesize) + "B " : "" )+ "\n";
        }
    } else {
        for(auto &item : fs::directory_iterator(path)) { // for standard answer
            reply += item.path().filename().string() + "\n";
        }
    }
    load_buffer(reply);
}

void sftpServer::cmd_cdir() {
    if(m_tquery.size() != 2 ) { // invalid arg count
        load_buffer("-Invalid query, usage: \"CDIR directory-path\"");
        return;
    }
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
    if(!fs::exists(path)) {
        load_buffer("-Can't connect to directory because: does not exist");
        return;
    }
    if(!fs::is_directory(path)) {
        load_buffer("-Can't connect to directory because: is not directory");
        return;
    }
    // TODO handle user not logged in, dostudovat, ako su tieto acct a pass pri tomto prikaze( a aj inych ) v RFCcku myslene
    //if(!m_logged_in) {
    //    load_buffer("+Directory ok, send account/password");
    //    return;
    //}
    m_wdir = fs::absolute(path);
    load_buffer("+Changed directory to " + m_wdir.string());
}

// TODO must be logged in first, VELMI NEBEZBEČNÝ KÓD - 🪦💀🪦💀🪦💀🪦💀🪦
void sftpServer::cmd_kill() {
    if(!is_valid_count(2,"-Invalid query, usage: \"KILL <file_spec>\"")) return;
    fs::path file_spec(m_tquery[1]);
    if(!m_logged_in) {
        load_buffer("-You must log in first");
        return;
    }
    /*
    if(fs::remove(file_spec)) {
        load_buffer("+" + file_spec.string() + " deleted");
        return;
    }
    load_buffer("-Not deleted");
     */
    load_buffer("-Command unavailable during debugging, SAFETY FIRST!");
}

// TODO toto je nebezpecne - neni nijak osetrene aky subor/folder sa bude menit, može vsetko dojebat
void sftpServer::cmd_name() {
    if(!is_valid_count(2, "-Invalid query, usage: \"NAME old-file-spec\"")) return;
    fs::path path(m_tquery[1]);
    if(!fs::exists(path)) {
        load_buffer("-Can't find " + path.string());
        return;
    }
    m_path_to_be_renamed = path;
    m_NAME = true;
    load_buffer("+File exists");
}

// TODO vylepsit - rozlisovat medzi file a folder
void sftpServer::cmd_tobe() {
    if(!is_valid_count(2, "-Invalid query, usage: \"TOBE new-file-spec\"")) return;
    if(!m_NAME) { // po NAME bol zadany iny prikaz
        load_buffer("-Send NAME query first");
        return;
    }
    fs::path new_name(m_tquery[1]);

    fs::path path_to_file = m_path_to_be_renamed.parent_path();
    PRINT2(m_path_to_be_renamed.string(), new_name.string());
    try {
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
    PRINT("in cmd_retr");
    if(!is_valid_count(2, "-Invalid query, usage: \"RETR file-spec\"")) return;
    fs::path file(m_tquery[1]);
    if(!fs::exists(file)) {
        load_buffer("-File does not exist");
        return;
    }
    size_t size = fs::file_size(file);
    m_retr_planned = true;
    m_retrieved_filename = file.string();
    m_retrieved_filesize = size;
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
    send_file(m_retrieved_filename);
    load_buffer("+File send successfully");
}

void sftpServer::cmd_stor() {
    if(!is_valid_count(3, "-Invalid query, usage: \"STOR { NEW | OLD | APP } file-spec\"")) return;
    auto type = m_tquery[1];
    auto filename = m_tquery[2];
    bool exists = fs::exists(fs::path(filename));
    m_stored_filename = filename;
    if(!(type == "NEW" || type == "OLD" || type == "APP")) {
        load_buffer("-Invalid query, usage: \"STOR { NEW | OLD | APP } file-spec\"");
        return;
    }
    if (type == "NEW") {
        if(exists) {
            load_buffer("-File exists, but system doesn't support generations");
            return;
        }
        load_buffer("+File does not exist, will create new file");
        m_stor_planned = true;
        //todo flag
        return;
    }
    m_stor_planned = true;
    if (type == "OLD") {
        if(exists) {
            load_buffer("+Will write over old file");
            return;
        }
        load_buffer("+Will create new file");
        return;
    }
    if(type == "APP") {
        if(exists) {
            load_buffer("+Will append to new file");
            return;
        }
        load_buffer("+Will create new file");
    }
}

void sftpServer::cmd_size() {
    if(!is_valid_count(2, "-Invalid query, usage: \"SIZE <number-of-bytes-in-file>\"")) return;
    if(!m_stor_planned) {
        load_buffer("-Send STOR query first");
        return;
    }
    if(!is_number(m_tquery[1])) {
        load_buffer("-Not a valid filesize");
        return;
    }
    m_stored_filesize = atoi(m_tquery[1].data());
    retrieve_file();
    load_buffer("+Saved " + m_stored_filename);
}


// =====================================================================================================================


void sftpServer::retrieve_file() {
    PRINT("in retrieve_file");
    //todo appending
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    int bytes_left = m_stored_filesize;
    ssize_t len;
    FILE *fp;


    fp = fopen(m_stored_filename.data(), "wb");
    if(fp == nullptr) {
        PRINT("Error with opening file");
        exit(0);
    }
    PRINT("serv: file opened");
    PRINT2("Bytes left: ", m_stored_filesize);

    while(bytes_left) {
        if(bytes_left < BUFFER_SIZE) {
            len = recv(m_socket, buffer, bytes_left, 0);
            fwrite(buffer, sizeof(char), len, fp);
            bytes_left -= len;
            printf("Received %lu bytes, expecting %d bytes\n", len, bytes_left);
            memset(buffer, 0, BUFFER_SIZE);
            break;
        } else {
            len = recv(m_socket, buffer, BUFFER_SIZE, 0); //256
            fwrite(buffer, sizeof(char), len, fp);
            bytes_left -= len;
            printf("Received %lu bytes, expecting: %d bytes\n", len, bytes_left);
        }
        memset(buffer, 0, BUFFER_SIZE);
    }
    fclose(fp);
    return;
}

void sftpServer::send_file(std::string filename) {
    FILE *fp;
    int n;
    int read_bytes;
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
    fclose(fp);
    return;
}

void sftpServer::check_tobe() {
    if(m_NAME) {
        if(m_tquery.front() != "TOBE") m_NAME = false;
    }
}

int sftpServer::receive() {
    int numbytes;
    memset(m_buffer, 0, BUFFER_SIZE); // clear buffer for query
    if((numbytes = recv(m_socket, m_buffer, BUFFER_SIZE-1, 0)) == -1) {
        PRINT("recv error, exiting...");
        exit(0);
    }
    return numbytes;
}

bool sftpServer::is_valid_count(int cnt, std::string msg, int upto) {
    int len = m_tquery.size();
    if(upto) {
        if(len < cnt || len > upto) {
            load_buffer(msg);
            return false;
        }
        return true;
    }
    if(len != cnt) {
        load_buffer(msg);
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
    bool ok = load_file(userpasses, "userpass.txt"); // load file
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