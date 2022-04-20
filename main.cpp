#include <string>
#include <iostream>
#include <filesystem>
#include <cstdlib>

#include "server.h"


#define SFTP_PORT 115
namespace fs = std::filesystem;


int main(int argc, char ** argv) {
    auto parser = ArgParserServer(argc, argv);
    parser.parse();
    auto server = sftpServer(&parser);
    server.start();

    return 0;
}



