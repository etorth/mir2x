/*
 * =====================================================================================
 *
 *       Filename: message.hpp
 *        Created: 01/24/2016 19:30:45
 *  Last Modified: 03/01/2016 00:22:29
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
    CM_PING,
    CM_LOGIN,
};

enum: uint8_t
{
    SM_PING,
    SM_LOGINOK,
    SM_LOGINFAIL,
};

#pragma pack(push, 1)
// client

typedef struct{
    char ID[16];
    char Password[16];
}CMLogin;

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
