#include <string>
#include <iostream>
#include <filesystem>
#include <cstdlib>

#include "argparser.h"
#include "server.h"


#define SFTP_PORT 115
namespace fs = std::filesystem;

void test() {
    system("pwd");
    system("cd client");
    system("pwd");
}


int main(int argc, char ** argv) {
    auto parser = ArgParserServer(argc, argv);
    auto server = sftpServer();
    parser.parse();
    server.start();
    return 0;
}
