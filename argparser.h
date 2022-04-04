#include <iostream>
#include <cstring>

#define SFTP_PORT 115
#define PRINT(x) std::cout << x << std::endl


class ArgParserServer {
    bool m_iFlag = false;
    bool m_pFlag = false;
    bool m_uFlag = false;
    bool m_fFlag = false;
    std::string m_iArg, m_pArg, m_uArg, m_fArg;
    int m_port = SFTP_PORT;
    int m_argc;
    char ** m_argv;


public:
    ArgParserServer(int argc, char ** argv);

    bool iFlag() const;

    bool pFlag() const;

    bool uFlag() const;

    bool fFlag() const;

    void printArgs() const;

    void validateArgs();

    void parse();

};