/*
 * =====================================================================================
 *
 *       Filename: argparser.hpp
 *        Created: 11/26/2018 07:47:59
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

inline bool FindOption(const argh::parser &rstArgParser, const std::string &szOpt)
{
    return rstArgParser[szOpt] || rstArgParser(szOpt);
}
