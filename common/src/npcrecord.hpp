/*
 * =====================================================================================
 *
 *       Filename: npcrecord.hpp
 *        Created: 11/23/2020 11:12:01
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
#include <utility>

class NPCRecord
{
    public:
        const char8_t *name;
        uint16_t lookID;
        int dirIndex;

    public:
        constexpr NPCRecord(const char8_t *argName, uint16_t argLookID, int argDirIndex)
            : name(argName ? argName : u8"")
            , lookID(argLookID)
            , dirIndex(argDirIndex)
        {}

    public:
        operator bool() const
        {
            return name[0] != '\0';
        }
};
