/*
 * =====================================================================================
 *
 *       Filename: clientmessage.hpp
 *        Created: 01/24/2016 19:30:45
 *  Last Modified: 04/04/2016 21:44:38
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

#pragma pack(push, 1)

typedef struct{
    char ID[16];
    char Password[16];
}CMLogin;
#pragma pack(pop)
