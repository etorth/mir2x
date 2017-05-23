/*
 * =====================================================================================
 *
 *       Filename: filesys.hpp
 *        Created: 02/08/2016 22:15:46
 *  Last Modified: 05/21/2017 01:23:42
 *
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
namespace FileSys
{
    bool MakeDir(const char *);
    bool RemoveDir(const char *);
    bool FileExist(const char *);
    bool DupFile(const char *, const char *);
}
