/*
 * =====================================================================================
 *
 *       Filename: servermessage.hpp
 *        Created: 01/24/2016 19:30:45
 *  Last Modified: 04/04/2016 22:02:48
 *
 *    Description: net message used by client and mono-server
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
#include <cstdint>

enum: uint8_t
{
    SM_PING,
    SM_LOGINOK,
    SM_LOGINFAIL,
};

#pragma pack(push, 1)

typedef struct{
    char     CharName[32];
    char     MapName[16];
    uint32_t UID;
    uint32_t SID;
    uint16_t Level;
    uint32_t MapX;
    uint32_t MapY;
    uint8_t  Direction;
}SMLoginOK;
#pragma pack(pop)
