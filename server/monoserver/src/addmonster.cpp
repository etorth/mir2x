/*
 * =====================================================================================
 *
 *       Filename: addmonster.cpp
 *        Created: 04/12/2016 19:07:52
 *  Last Modified: 05/01/2016 23:23:33
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

bool MonoServer::AddMonster(uint32_t nMonsterInex, uint32_t nMapID, int nX, int nY, bool bStrict)
{
    AMAddMonster stAMAM;

    stAMAM.MonsterIndex = nMonsterInex;
    stAMAM.MapID        = nMapID;
    stAMAM.X            = nX;
    stAMAM.Y            = nY;
    stAMAM.Strict       = bStrict;

    MessagePack stResponseMPK;
    if(!Send(MessagePack(MPK_ADDMONSTER, stAMAM), m_ServiceCoreAddress, &stResponseMPK)){
        // sent and received
        AddLog(LOGTYPE_WARNING, "message sent for adding monster failed");
        return false;
    }

    switch(stResponseMPK.Type()){
        case MPK_OK:
            {
                AddLog(LOGTYPE_INFO, "monster with index = %d added", nMonsterInex);
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
