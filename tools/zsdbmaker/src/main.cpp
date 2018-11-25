/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 07/20/2017 00:34:13
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *
 * =====================================================================================
 */
#include <cstdio>
#include "zsdb.hpp"

int main(int argc, char *argv[])
{
    if(argc < 2){
        std::printf("Usage: zsdbmaker dst_path src_path\n\n");
    }

    if(!ZSDB::BuildDB(argv[1], nullptr, argv[2], nullptr, 0.90)){
        std::printf("create zsdb failed...\n");
    }

    return 0;
}
