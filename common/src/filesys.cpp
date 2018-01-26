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

#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#include <direct.h>
#include <io.h>
#else
#include<sys/stat.h>
#include<unistd.h>
#endif

#include <string>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dirent.h>

#include "filesys.hpp"

bool FileSys::RemoveDir(const char *szAbsolutePath)
{
    std::string szPathName = szAbsolutePath;
    if(szPathName.back() != '/'){
        szPathName += "/";
    }

    auto stDir = opendir(szPathName.c_str());
    if(stDir == nullptr){
        return false;
    }

    struct dirent *stDirItem = nullptr;
    while((stDirItem = readdir(stDir))!= nullptr){
        if(!std::strcmp(stDirItem->d_name, ".")){
            continue;
        }
        if(!std::strcmp(stDirItem->d_name, "..")){
            continue;
        }

        if(stDirItem->d_type == DT_DIR){
            if(!RemoveDir((szPathName + stDirItem->d_name).c_str())){
                return false;
            }else{
                rewinddir(stDir);
                continue;
            }
        }

        // not std::remove(), they are different functions
        if(remove((szPathName + stDirItem->d_name).c_str()) != 0){
            return false;
		}else{
			rewinddir(stDir);
			continue;
		}
    }
    return !closedir(stDir) &&
#ifdef _WIN32
        !_rmdir
#else
        !rmdir
#endif
        (szPathName.c_str());

}

bool FileSys::MakeDir(const char *szDirName)
{
    return 
#ifdef _WIN32
        !_mkdir(szDirName);
#else
        !mkdir(szDirName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

bool FileSys::FileExist(const char *szFileName)
{
    return 
#ifdef _WIN32
        !_access(szFileName, 0)
#else
        !access(szFileName, F_OK)
#endif
        ;
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
