/*
 * =====================================================================================
 *
 *       Filename: addplayer.cpp
 *        Created: 04/06/2016 18:44:28
 *  Last Modified: 04/23/2016 02:14:46
 *
 *    Description: active a player in online mode, player GUID is used
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

#include "session.hpp"
#include "monoserver.hpp"
#include "serverconfigurewindow.hpp"

bool MonoServer::AddPlayer(uint32_t nSID, uint32_t nUID)
{
    uint32_t nAddTime = GetTimeTick();
    uint64_t nKey = ((uint64_t)nUID << 32) + nUID;



    return true;
}
