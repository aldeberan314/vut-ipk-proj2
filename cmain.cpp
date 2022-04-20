//
// Created by Andrej Hyros on 08/04/2022.
//

#include "client.h"

int main(int argc, char *argv[]) {

    auto parser = ArgParserClient(argc, argv);
    parser.parse();
    auto client = sftpClient(&parser);
    client.start();
}