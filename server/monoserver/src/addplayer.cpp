/*
 * =====================================================================================
 *
 *       Filename: addplayer.cpp
 *        Created: 04/12/2016 19:07:52
 *  Last Modified: 05/20/2016 17:06:29
 *
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

#include "log.hpp"
#include "taskhub.hpp"
#include "sysconst.hpp"
#include "monoserver.hpp"
#include "messagepack.hpp"

bool MonoServer::AddPlayer(uint32_t nGUID, uint32_t nMapID, int nX, int nY)
{
    AMAddMonster stAMAM;

    stAMAM.GUID    = nGUID;
    stAMAM.MapID   = nMapID;
    stAMAM.UID     = GetUID();
    stAMAM.AddTime = GetTickCount();

    stAMAM.Strict = bStrict;
    stAMAM.X      = nX;
    stAMAM.Y      = nY;
    stAMAM.R      = 10;

    MessagePack stResponseMPK;
    if(Forward(MessageBuf(MPK_ADDPLAYER, stAMAM), m_ServiceCoreAddress, &stResponseMPK)){
        // sent and received
        AddLog(LOGTYPE_WARNING, "message sent for adding monster failed");
        return false;
    }

    switch(stResponseMPK.Type()){
        case MPK_OK:
            {
                AddLog(LOGTYPE_INFO, "monster with index = %d added", nGUID);
                return true;
            }
        default:
            {
                break;
            }
    }

    AddLog(LOGTYPE_INFO, "adding monster failed");
    return false;
}
