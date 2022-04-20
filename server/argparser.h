#include <iostream>
#include <cstring>
#include "../utils/error.h"

#define SFTP_PORT "115"
#define PRINT(x) std::cout << x << std::endl

/**
 * Servers argument parser
 */
class ArgParserServer {
public:
    bool m_iFlag = false;
    bool m_pFlag = false;
    bool m_uFlag = false;
    bool m_fFlag = false;
    std::string m_iArg, m_uArg, m_fArg, m_pArg = SFTP_PORT;
    int m_port = 115;
    int m_argc;
    char ** m_argv;


public:
    ArgParserServer(int argc, char ** argv);

    /**
     * getters for arg flags
     * @return
     */
    bool iFlag() const;

    bool pFlag() const;

    bool uFlag() const;

    bool fFlag() const;

    /**
     * Prints input args
     */
    void printArgs() const;

    /**
     * Validates input args
     */
    void validateArgs();

    /**
     * Parses input args
     */
    void parse();

};