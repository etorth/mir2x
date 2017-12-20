/*
 * =====================================================================================
 *
 *       Filename: servicecore.cpp
 *        Created: 04/22/2016 18:16:53
 *  Last Modified: 12/20/2017 00:13:41
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

#include <cstring>
#include <system_error>

#include "player.hpp"
#include "actorpod.hpp"
#include "metronome.hpp"
#include "mapbindbn.hpp"
#include "monoserver.hpp"
#include "servicecore.hpp"

ServiceCore::ServiceCore()
    : ActiveObject()
    , m_MapList()
{
    auto fnRegisterClass = [this]()
    {
        if(!RegisterClass<ServiceCore, ActiveObject>()){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Class registration for <ServiceCore, ActiveObject> failed");
            g_MonoServer->Restart();
        }
    };
    static std::once_flag stFlag;
    std::call_once(stFlag, fnRegisterClass);
}

void ServiceCore::OperateAM(const MessagePack &rstMPK, const Theron::Address &rstAddr)
{
    switch(rstMPK.Type()){
        case MPK_BADSESSION:
            {
                On_MPK_BADSESSION(rstMPK, rstAddr);
                break;
            }
        case MPK_LOGINQUERYDB:
            {
                On_MPK_LOGINQUERYDB(rstMPK, rstAddr);
                break;
            }
        case MPK_ADDCHAROBJECT:
            {
                On_MPK_ADDCHAROBJECT(rstMPK, rstAddr);
                break;
            }
        case MPK_TRYMAPSWITCH:
            {
                On_MPK_TRYMAPSWITCH(rstMPK, rstAddr);
                break;
            }
        case MPK_NEWCONNECTION:
            {
                On_MPK_NEWCONNECTION(rstMPK, rstAddr);
                break;
            }
        case MPK_NETPACKAGE:
            {
                On_MPK_NETPACKAGE(rstMPK, rstAddr);
                break;
            }
        case MPK_QUERYMAPLIST:
            {
                On_MPK_QUERYMAPLIST(rstMPK, rstAddr);
                break;
            }
        case MPK_QUERYCOCOUNT:
            {
                On_MPK_QUERYCOCOUNT(rstMPK, rstAddr);
                break;
            }
        case MPK_QUERYMAPUID:
            {
                On_MPK_QUERYMAPUID(rstMPK, rstAddr);
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported message: %s", rstMPK.Name());
                g_MonoServer->Restart();
                break;
            }
    }
}

void ServiceCore::OperateNet(uint32_t nSID, uint8_t nType, const uint8_t *pData, size_t nDataLen)
{
    switch(nType){
        case CM_LOGIN:
            {
                Net_CM_Login(nSID, nType, pData, nDataLen);
                break;
            }
        default:
            {
                break;
            }
    }
}

bool ServiceCore::LoadMap(uint32_t nMapID)
{
    if(nMapID){
        if(m_MapList.find(nMapID) == m_MapList.end()){
            extern MapBinDBN *g_MapBinDBN;
            if(g_MapBinDBN->Retrieve(nMapID)){
                auto pMap = new ServerMap(this, nMapID);
                pMap->Activate();
                m_MapList[nMapID] = pMap;
                return true;
            }else{
                return false;
            }
        }
        return true;
    }
    return false;
}

const ServerMap *ServiceCore::RetrieveMap(uint32_t nMapID)
{
    if(nMapID){
        auto pMap = m_MapList.find(nMapID);
        if(pMap == m_MapList.end()){
            if(LoadMap(nMapID)){
                pMap = m_MapList.find(nMapID);
            }
        }

        return (pMap == m_MapList.end()) ? nullptr : pMap->second;
    }
    return nullptr;
}
