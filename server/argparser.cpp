#include "argparser.h"


    ArgParserServer::ArgParserServer(int argc, char ** argv) {
        m_argc = argc;
        m_argv = argv;
    }

    bool ArgParserServer::iFlag() const {
        return m_iFlag;
    }

    bool ArgParserServer::pFlag() const {
        return m_pFlag;
    }

    bool ArgParserServer::uFlag() const {
        return m_uFlag;
    }

    bool ArgParserServer::fFlag() const {
        return m_fFlag;
    }

    void ArgParserServer::printArgs() const {
        std::cout << "ipuf: " << m_iFlag << m_pFlag << m_uFlag << m_fFlag << std::endl;
        PRINT(m_iArg);
        PRINT(m_pArg);
        PRINT(m_uArg);
        PRINT(m_fArg);
    }

    void ArgParserServer::validateArgs() {
        if (!m_uFlag or !m_fFlag) {
            error_call(CMD_ARGUMENT_ERROR, "Error: -h or -f missing, exiting");
        }
        if (m_pFlag) {
            m_port = stoi(m_pArg);
        }
        //PRINT("[ARGPARSER] ALL GOOD");
    }

    void ArgParserServer::parse() {
        //PRINT("Parsing command line arguments");
        for (int i = 1; i < m_argc; i+=2) {
            if (!strcmp(m_argv[i], "-i")) {
                m_iFlag = true;
                m_iArg = m_argv[i+1];
            } else
            if (!strcmp(m_argv[i], "-p")) {
                m_pFlag = true;
                m_pArg = m_argv[i+1];
            } else
            if (!strcmp(m_argv[i], "-u")) {
                m_uFlag = true;
                m_uArg = m_argv[i+1];
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