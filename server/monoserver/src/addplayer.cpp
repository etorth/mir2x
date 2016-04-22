/*
 * =====================================================================================
 *
 *       Filename: addplayer.cpp
 *        Created: 04/06/2016 18:44:28
 *  Last Modified: 04/22/2016 15:27:58
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

bool MonoServer::AddPlayer(int nSID, uint32_t)
{
    // auto pSession = m_SessionIO->Validate(nSessionID);
    // if(!pSession){
    //     AddLog(LOGTYPE_INFO, "session lost, ignore login requestf for (%d:%d)", nSessionID, nGUID);
    //     return false;
    // }
    //
    // {
    //     std::lock_guard<std::mutex> stLockGuard(m_OnlinePlayerVLock);
    //     extern ServerConfigureWindow *g_ServerConfigureWindow;
    //     if(m_OnlinePlayerV.size() > (size_t)g_ServerConfigureWindow->MaxPlayerCount()){
    //         pSession->Send(SM_SERVERFULL, [this, nSessionID](){ m_SessionIO->Kill(nSessionID); });
    //         return false;
    //     }
    // }
    return true;
}
