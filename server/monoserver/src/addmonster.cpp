/*
 * =====================================================================================
 *
 *       Filename: addmonster.cpp
 *        Created: 04/12/2016 19:07:52
 *  Last Modified: 05/26/2016 17:08:10
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
#include "monster.hpp"
#include "taskhub.hpp"
#include "sysconst.hpp"
#include "monoserver.hpp"
#include "syncdriver.hpp"
#include "messagepack.hpp"

bool MonoServer::AddMonster(uint32_t nMonsterID, uint32_t nMapID, int nX, int nY, bool bAllowVoid)
{
    AMAddCharObject stAMACO;
    stAMACO.Type = OBJECT_MONSTER;

    stAMACO.Common.MapID     = nMapID;
    stAMACO.Common.MapX      = nX;
    stAMACO.Common.MapY      = nY;
    stAMACO.Common.R         = 10; // TODO
    stAMACO.Common.AllowVoid = bAllowVoid;

    stAMACO.Monster.MonsterID = nMonsterID;

    // TODO
    // do we need the response? I haven't decide yet, but I prefer yes
    MessagePack stResponseMPK;
    if(SyncDriver().Forward({MPK_ADDCHAROBJECT, stAMACO}, m_ServiceCoreAddress, &stResponseMPK)){
        // sent and received
        AddLog(LOGTYPE_WARNING, "message sent for adding monster failed");
        return false;
    }

    switch(stResponseMPK.Type()){
        case MPK_OK:
            {
                AddLog(LOGTYPE_INFO, "monster with index = %d added", nMonsterID);
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
