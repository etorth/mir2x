/*
 * =====================================================================================
 *
 *       Filename: stackdump.hpp
 *        Created: 01/20/2018 20:19:36
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

#include <vector>
#include <string>

namespace StackDump
{
    std::vector<std::string> Dump(size_t);
}
