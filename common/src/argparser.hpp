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

class arg_parser: public argh::parser
{
    public:
        arg_parser(int argc, char *argv[])
            : argh::parser(argc, argv)
        {}
};
