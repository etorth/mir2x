/*
 * =====================================================================================
 *
 *       Filename: addmonster.cpp
 *        Created: 04/12/2016 19:07:52
 *  Last Modified: 04/13/2016 23:53:56
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

// create an specified monster at specified place
// parameters:
//      nMonsterInex:
//      nMapID      :   
//      nX          :   pixel location x
//      nY          :   pixel location y
//      bStrict     :   true for exact location, otherwise use location randomly picked
//      pUID        :
//      pAddTime    :
bool MonoServer::AddMonster(uint32_t nMonsterInex, uint32_t nMapID,
        int nX, int nY, bool bStrict, uint32_t *pUID, uint32_t *pAddTime)
{
    // 1. check argument
    if(nMonsterInex == 0 || nMapID == 0){ return false; }

    // 2. check location
    // TODO do we need a map-wise lock? currently I am negative to it
    ServerMap *pMap = nullptr;
    {
        std::lock_guard<std::mutex> stLockGuard(m_MapVLock);
        auto stInst = m_MapV.find(nMapID);
        if(stInst == m_MapV.end()){
            // try to create an new map, may throw for invalid argument
            // TODO is std::unordered_map exception-safe?
            try{
                m_MapV.emplace(nMapID, std::make_shared<ServerMap>(nMapID));
            }catch(...){
                return false;
            }
        }
        stInst = m_MapV.find(nMapID);
        if(stInst == m_MapV.end()){
            extern Log *g_Log;
            g_Log->AddLog(LOGTYPE_WARNING, "map error map id = %d", nMapID);
            return false;
        }

        pMap = stInst->second.get();
    }

    if(!pMap || !pMap->ValidP(nX, nY)){ return false; }

    // 3. try to add to map
    uint32_t nUID     = m_ObjectUID++;
    uint32_t nAddTime = GetTickCount();

    // try-catch
    Monster *pMonster = nullptr;
    try{
        switch(nMonsterInex){
            case MONSTER_DEER:
            default:
                {
                    pMonster = new Monster(nMonsterInex, nUID, nAddTime);
                }
        }
    }catch(...){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_WARNING,
                "add monster with index %d at map %d of location (%d, %d) failed",
                nMonsterInex, nMapID, nX, nY);
        return false;
    }

    // prepare the 64-bit key
    uint64_t nKey = ((uint64_t)nUID << 32) + nAddTime;

    // now we have a valid monster object with state STATE_EMBRYO
    if(!m_CharObjectHub.Add(nKey, (CharObject *)pMonster, [](CharObject *pResource){ delete pResource; })){
        delete pMonster;

        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_WARNING,
                "create monster with index %d failed", nMonsterInex);
        return false;
    }

    // now the monster object is owned by the hub, if we need it
    // we should retrieve from the hub...
    auto pGuard = CheckOut<CharObject, true>(nUID, nAddTime);
    if(!pGuard){
        // strange error, check your logic...
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_WARNING, "strange error, may contain serious logic error..");
        Remove<CharObject>(nUID, nAddTime);
        return false;
    }

    int nTryLoop = (bStrict ? 1 : 100);
    while(nTryLoop--){
        if(pMap->ObjectMove(nX, nY, pGuard.Get())){
            return true;
        }
        nX = std::rand() % (pMap->W() * SYS_MAPGRIDXP);
        nY = std::rand() % (pMap->H() * SYS_MAPGRIDYP);
    }

    if(pUID){ *pUID = nUID; }
    if(pAddTime){ *pAddTime = nAddTime; }

    // now this object is ready
    pGuard->SetState(STATE_INCARNATED, true);

    extern TaskHub *g_TaskHub;
    auto fnOperate = [this, nUID, nAddTime]()
    {
        Operate(nUID, nAddTime);
    };

    g_TaskHub->Add(fnOperate);

    return true;
}
