//
// Created by Andrej Hyros on 08/04/2022.
//

#include "client.h"

int main(int argc, char *argv[]) {
    auto client = sftpClient();
    client.start();
}