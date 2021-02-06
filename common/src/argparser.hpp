/*
 * =====================================================================================
 *
 *       Filename: argparser.hpp
 *        Created: 11/27/2018 07:49:02
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include "argh.h"
#include "strf.hpp"

class arg_parser: public argh::parser
{
    public:
        arg_parser(char *argv[])
            : argh::parser(argv)
        {}

        arg_parser(int argc, char *argv[])
            : argh::parser(argc, argv)
        {}

    public:
        int has_option(const std::string &opt) const
        {
            auto flag = (bool)((*this)[opt]);
            auto parm = (bool)((*this)(opt));

            if(flag && parm){
                return 2;
            }

            if(flag || parm){
                return 1;
            }
            return 0;
        }

        // support
        // --opt      : return true
        // --opt=[xx] : return true with warning message

        bool has_flag(const std::string &opt) const
        {
            if((*this)(opt)){
                print_message(str_printf("--%s ignores its value: %s", opt.c_str(), (*this)(opt).str().c_str()));
                return true;
            }
            return (*this)[opt];
        }

        // accept
        // --param     : return default
        // --param=    : return default
        // --param=abc : return abc

        std::string has_param(const std::string &opt) const
        {
            return (*this)(opt).str();
        }

    protected:
        virtual void print_message(const std::string &) const
        {
        }
};
