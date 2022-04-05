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
        error_call(CONNECTION_ERROR, "gai_err", 1);
    }

    PRINT("[SERVER] Waiting for connections...");
    sock = bind_to(p, yes, servinfo);
    freeaddrinfo(servinfo);
    if(!p) {
        error_call(CONNECTION_ERROR, "server:failed to bind", errno);
    }

    if(listen(sock, BACKLOG) == -1) {
        error_call(CONNECTION_ERROR, "server: listen", errno);
    }

    m_socket = accept(sock, (sockaddr*)&m_client_addr, &sin_size);
    if(m_socket == -1) {
        error_call(CONNECTION_ERROR, "server: accept");
    }

    inet_ntop(m_client_addr.ss_family, get_in_addr((sockaddr*)&m_client_addr), m_ipaddr, sizeof(m_ipaddr));
    PRINT2("server: got connection from", m_ipaddr);

    start_conversation();
}

void sftpServer::close_connection() {
    close(m_socket);
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
            break;
        }
        //parse query
        parse_query();

        //respond
        //PRINT2(strlen(m_buffer), m_buffer);
        PRINT(m_buffer);
        send(m_socket, m_buffer, strlen(m_buffer), 0);
        if(m_done) break;
    }

    close_connection();
}

void sftpServer::parse_query() {
    std::string query(m_buffer);
    memset(m_buffer, 0, BUFFER_SIZE); // clear buffer for reply
    const char delimiter = ' ';
    tokenize(query, delimiter, m_tquery);
    check_tobe();

    switch (hash_string(m_tquery.front())) {
        case USER:
            cmd_user();
            break;
        case ACCT:
            cmd_acct();
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
            break;
        case STOR:
            break;
        case TOBE:
            cmd_tobe();
            break;
        case ERROR:
        default:
            load_buffer("-Invalid query");
            break;
    }
    m_tquery.clear();
}






void sftpServer::cmd_user() {
    if(!is_valid_count(2,"-Invalid query, usage: \"USER <userid>\"")) return;
    auto user_id = m_tquery[1];
    if(is_valid_user(user_id)) { // user is valid
        load_buffer("+" + m_userid + " valid, send account and password");
        m_userid_sent = true;
        return;
    }
    load_buffer("-Invalid user-id, try again"); // self-explanatory
}

void sftpServer::cmd_acct() {
    if(!is_valid_count(2,"-Invalid query, usage: \"ACCT <account>\"")) return;
    if(m_userid == m_tquery[1]) { // account is valid
        load_buffer("+Account valid, send password");
        m_acc_sent = true;
        return;
    }
    load_buffer("-Invalid account, try again"); // self-explanatory
}

// TODO - preskumat postupnost prikazov USER - PASS - ACCT - PASS a najst riesenie ktore dava zmysel. Momentalne nedava vypisok zmysel, pretoze po pyta heslo znovu
// mozno zakazat posielanie dalsieho hesla ked uz sme logged in? napr "+Already logged in"
// TODO pri prikazoch kontrolovat ci je user logged in, nejaku fciu si na to napisat
void sftpServer::cmd_pass() {
    if(!is_valid_count(2,"-Invalid query, usage: \"PASS <password>\"")) return;
    if(!m_userid_sent) { // userid was not providing but already sending password
        load_buffer("-First send username");
        return;
    }
    if(m_tquery[1] == m_password) { // password is correct
        if(!m_acc_sent) { // password is correct but user did not sent account
            load_buffer("+Send account");
        } else load_buffer("!" + m_userid + " logged in");
        m_logged_in = true;
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
        if(path.string() == "..") { // user passed ..
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

// TODO must be logged in first
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

void sftpServer::cmd_tobe() {
    if(!is_valid_count(2, "-Invalid query, usage: \"TOBE new-file-spec\"")) return;
    fs::path new_name(m_tquery[1]);

    fs::path path_to_file = m_path_to_be_renamed.parent_path();
    PRINT2(m_path_to_be_renamed.string(), new_name.string());
    fs::rename(m_path_to_be_renamed, path_to_file/new_name);
    load_buffer( "+" + m_path_to_be_renamed.string() + " renamed to " + new_name.string());
}

void sftpServer::cmd_done() {
    load_buffer("+OK, bye!");
    m_done = true;
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

bool sftpServer::is_valid_user(std::string userid) {
    std::vector<std::string> userpasses; // holds all lines from userpass.txt
    std::vector<std::string> pair; // will hold user and pass of single line
    bool ok = load_file(userpasses, "userpass.txt"); // load file
    if(!ok) error_call(FILE_IO_ERROR, "Loading userpass.txt failed"); // error
    for(auto user : userpasses) { // iterate and find match
        tokenize(user, ':', pair);
        if(userid == pair.front()) { // match
            m_userid = userid; // user is valid, save userid and password to object
            m_password = pair.back();
            return true;
        }
        pair.clear();
    }
    return false;
}
