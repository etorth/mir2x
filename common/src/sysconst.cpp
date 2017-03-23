/*
 * =====================================================================================
 *
 *       Filename: sysconst.cpp
 *        Created: 06/02/2016 11:43:04
 *  Last Modified: 03/23/2017 11:27:46
 *
 *    Description: don't refer to monoserver->AddLog() or something
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

#include <cstdint>
#include <cstddef>
#include "sysconst.hpp"

// I need to make sure for array access range
static size_t g_MaxValidMapID = 0;
const static char *g_MapName[] = {
    nullptr,        // 0
    "DESC.BIN",     // 1
    nullptr,
};

const char *SYS_MAPNAME(uint32_t nMapID)
{
    return "DESC.BIN";

    if(!g_MaxValidMapID){
        g_MaxValidMapID = 1;
        auto pName = g_MapName + 1;
        while(*pName){
            pName++;
            g_MaxValidMapID++;
        }
    }

    if(nMapID < g_MaxValidMapID){
        return g_MapName[g_MaxValidMapID];
    }

    return nullptr;
}
