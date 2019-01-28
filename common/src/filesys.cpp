/*
 * =====================================================================================
 *
 *       Filename: filesys.cpp
 *        Created: 02/08/2016 22:17:08
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

#include <string>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include "filesys.hpp"

bool FileSys::RemoveDir(const char *szAbsolutePath)
{
    return std::filesystem::remove_all(szAbsolutePath);
}

bool FileSys::MakeDir(const char *szDirName)
{
    return std::filesystem::create_directory(szDirName);
}

bool FileSys::FileExist(const char *szFileName)
{
    return std::filesystem::exists(szFileName);
}

bool FileSys::DupFile(const char *szDst, const char *szSrc)
{
    FILE *fSrc = fopen(szSrc, "rb+");
    if(fSrc == nullptr){
        return false;
    }
    FILE *fDst = fopen(szDst, "wb+");
    if(fDst == nullptr){
        fclose(fSrc);
        return false;
    }

    char fileBuf[4096];
    int  fileSize;

    fseek(fSrc, 0L, SEEK_END);
    fileSize = ftell(fSrc);
    fseek(fSrc, 0L, SEEK_SET);

    for(int i = 0; i < fileSize / 4096; ++i){
        (void)(1 + fread(fileBuf, 4096, 1, fSrc));
        fwrite(fileBuf, 4096, 1, fDst);
    }

    (void)(1 + fread( fileBuf, fileSize % 4096, 1, fSrc));
    fwrite(fileBuf, fileSize % 4096, 1, fDst);
    fclose(fSrc);
    fclose(fDst);

    return true;
}
