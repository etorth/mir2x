/*
 * =====================================================================================
 *
 *       Filename: addmonster.cpp
 *        Created: 04/12/2016 19:07:52
 *  Last Modified: 04/12/2016 21:59:51
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

#include "monoserver.hpp"

bool MonoServer::AddMonster(uint32_t nMonsterInex, uint32_t nMapID, int nX, int nY)
{
}

bool MonoServer::AddMonster(uint32_t nMonsterInex,
        uint32_t nMapID, uint32_t *pUID, uint32_t *pAddTime)
{

}

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
        int nX, int nY, bool bStrict, uint32_t *pUID = nullptr, uint32_t *pAddTime = nullptr)
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
                m_MapV[nMapID] = std::make_shared<ServerMap>(nMapID);
            }catch(...){
                return false;
            }
        }
        stInst = m_MapV.find(nMapID);
        if(stInst == m_MapV.end()){ return false; }

        pMap = stInst.second.get();
    }

    if(!pMap || !pMap->ValidP(nX, nY)){ return false; }

    // 3. try to add to map
    {
        uint32_t nUID     = m_ObjectUID++;
        uint32_t nAddTime = GetTickCount();

        // try-catch
        auto pMonster = std::make_shared<Monster>(nMonsterInex, nUID, nAddTime);

        int nTryLoop = (bStrict ? 1 : 100);
        while(nTryLoop--){
            if(pMap->AddObject(nX, nY, pMonster)){
                return true;
            }
            nX = std::rand() % (pMap->W() * SYS_MAPXP);
            nY = std::rand() % (pMap->H() * SYS_MAPYP);
        }

        return false;



        if(ObjectMove(nX, nY, pMonster)){
            if(m_CharObjectHub.Add(((uint64_t)nUID << 32) + nAddTime, pMonster))
        }

    }


    int nGridX = nX / SYS_MAPXP;
    int nGridY = nY / SYS_MAPYP;
}
