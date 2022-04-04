#include <string>
#include <iostream>

#include "argparser.h"
#include "server.h"


#define SFTP_PORT 115


int main(int argc, char ** argv) {
    auto parser = ArgParserServer(argc, argv);
    auto server = sftpServer();
    parser.parse();
    server.start();
    return 0;
}
