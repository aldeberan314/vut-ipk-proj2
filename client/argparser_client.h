//
// Created by Andrej Hyros on 09/04/2022.
//
#include <iostream>
#include <cstring>

#include "../utils/error.h"

/**
 * Clients argument parser
 */
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

    /**
     * getters for arg flags
     * @return
     */
    bool iFlag() const;

    bool pFlag() const;

    bool uFlag() const;

    bool fFlag() const;

    /**
     * prints args
     */
    void printArgs() const;

    /**
     * validates input args
     */
    void validateArgs();

    /**
     * parses input args
     */
    void parse();

};



