//
// Created by Andrej Hyros on 09/04/2022.
//

#include "argparser_client.h"

#define PRINT(x) std::cout << x << std::endl

ArgParserClient::ArgParserClient(int argc, char ** argv) {
    m_argc = argc;
    m_argv = argv;
}

void ArgParserClient::validateArgs() {
    if (!m_hFlag or !m_fFlag) {
        error_call(CMD_ARGUMENT_ERROR, "Error: -h or -f missing, exiting");
    }
    if (m_pFlag) {
        m_port = stoi(m_pArg);
    }
    //PRINT("[ARGPARSER] ALL GOOD");
}

void ArgParserClient::parse() {
    //PRINT("Parsing command line arguments");
    for (int i = 1; i < m_argc; i+=2) {
        //LOG(DEBUG) << "parsing" << m_argv[i];
        if (!strcmp(m_argv[i], "-h")) {
            m_hFlag = true;
            m_hArg = m_argv[i+1];
        } else
        if (!strcmp(m_argv[i], "-p")) {
            m_pFlag = true;
            m_pArg = m_argv[i+1];
        } else
        if (!strcmp(m_argv[i], "-f")) {
            m_fFlag = true;
            m_fArg = m_argv[i+1];
        } else {
            error_call(CMD_ARGUMENT_ERROR, "Error: unknown option argument " + std::string(m_argv[i]) + ", exiting");
        }
    }
    //printArgs();
    validateArgs();
}