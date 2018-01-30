/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 08/31/2017 16:12:32
 *    Description: convert a file name to its code
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

#include <cstdio>
#include <cstring>
#include "dbcomid.hpp"

int main(int argc, char *argv[])
{
    if(argc == 2){
        if(auto nMapID = DBCOM_MAPID(argv[1])){
            std::printf("%08X", nMapID);
            return 0;
        }
    }
    return 1;
}
