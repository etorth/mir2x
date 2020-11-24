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
namespace filesys
{
    bool   makeDir(const char *);
    bool removeDir(const char *);

    bool  hasFile(const char *);
    void copyFile(const char *, const char *);
}
