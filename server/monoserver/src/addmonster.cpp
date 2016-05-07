/*
 * =====================================================================================
 *
 *       Filename: addmonster.cpp
 *        Created: 04/12/2016 19:07:52
 *  Last Modified: 05/07/2016 03:18:18
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

bool MonoServer::AddMonster(uint32_t nGUID, uint32_t nMapID, int nX, int nY, bool bStrict)
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
    if(Send(MessageBuf(MPK_ADDMONSTER, stAMAM), m_ServiceCoreAddress, &stResponseMPK)){
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
