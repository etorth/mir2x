/*
 * =====================================================================================
 *
 *       Filename: filesys.hpp
 *        Created: 02/08/2016 22:15:46
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
#include <tuple>
#include <string>
#include <vector>

namespace filesys
{
    bool    hasDir(const char *);
    bool   makeDir(const char *);
    bool removeDir(const char *);

    bool  hasFile(const char *);
    void copyFile(const char *, const char *);

    std::vector<std::string> getFileList  (const char *, bool /* fullPath */, const char * /* reg */ = nullptr);
    std::vector<std::string> getSubDirList(const char *, bool /* fullPath */, const char * /* reg */ = nullptr);

    std::tuple<std::string, std::string> decompFileName(const char *);
}
