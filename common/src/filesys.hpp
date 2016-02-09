/*
 * =====================================================================================
 *
 *       Filename: filesys.hpp
 *        Created: 02/08/2016 22:15:46
 *  Last Modified: 02/08/2016 22:17:06
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

bool DupFile(const char *, const char *);
bool RemoveDir(const char *);
bool MakeDir(const char *);
bool FileExist(const char *);
