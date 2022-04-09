//
// Created by Andrej Hyros on 09/04/2022.
//
#include <iostream>
#include <cstring>

class ArgParserClient {
public:

    int m_port = 115;
    int m_argc;
    char ** m_argv;


    bool m_hFlag = false;
    bool m_pFlag = false;
    bool m_fFlag = false;
    std::string m_hArg, m_fArg, m_pArg = "115";

public:
    ArgParserClient(int argc, char ** argv);

    bool iFlag() const;

    bool pFlag() const;

    bool uFlag() const;

    bool fFlag() const;

    void printArgs() const;

    void validateArgs();

    void parse();

};



