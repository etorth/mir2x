/*
 * =====================================================================================
 *
 *       Filename: sysconst.cpp
 *        Created: 06/02/2016 11:43:04
 *  Last Modified: 04/01/2017 19:29:17
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

#include <string>
#include <cstdint>
#include <cstddef>
#include <unordered_map>

#include "sysconst.hpp"

typedef struct _MapRecord
{
    const std::string Name;
    const std::string FileName;

    _MapRecord(const char *szName, const char *szFileName)
        : Name(szName)
        , FileName(szFileName)
    {}
}MapRecord;

const static std::unordered_map<uint32_t, MapRecord> s_MapRecord = {
    {0,  {"", ""}},
    {1,  {"test", "DESC.BIN"}}};

const char *SYS_MAPNAME(uint32_t nMapID)
{
    if(nMapID){
        if(s_MapRecord.find(nMapID) != s_MapRecord.end()){
            return s_MapRecord.at(nMapID).Name.c_str();
        }
    }

    return nullptr;
}

const char *SYS_MAPFILENAME(uint32_t nMapID)
{
    if(nMapID){
        if(s_MapRecord.find(nMapID) != s_MapRecord.end()){
            return s_MapRecord.at(nMapID).FileName.c_str();
        }
    }

    return nullptr;
}
