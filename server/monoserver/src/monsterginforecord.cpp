/*
 * =====================================================================================
 *
 *       Filename: monsterginforecord.cpp
 *        Created: 06/13/2016 23:52:58
 *  Last Modified: 06/14/2016 00:20:15
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
#include "monsterginforecord.hpp"

MonsterGInfoRecord::MonsterGInfoRecord(uint32_t nMonsterID,
        uint32_t nLookID0,
        uint32_t nLookID1,
        uint32_t nLookID2,
        uint32_t nLookID3,
        uint32_t nR0,
        uint32_t nR1,
        uint32_t nR2,
        uint32_t nR3)
{
    m_MonsterID  = nMonsterID;

    m_LookIDV[0] = nLookID0;
    m_LookIDV[1] = nLookID1;
    m_LookIDV[2] = nLookID2;
    m_LookIDV[3] = nLookID3;

    m_RV[0]      = nR0;
    m_RV[1]      = nR1;
    m_RV[2]      = nR2;
    m_RV[3]      = nR3;
}

uint32_t MonsterGInfoRecord::LookID(int nLookIDV) const
{
    if(nLookIDV >= 0 && nLookIDV < 4){
        return m_LookIDV[nLookIDV];
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid argument");
    g_MonoServer->Restart();

    return 0;
}

uint32_t MonsterGInfoRecord::R(int nLookIDV) const
{
    if(nLookIDV >= 0 && nLookIDV < 4){
        return m_RV[nLookIDV];
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid argument");
    g_MonoServer->Restart();

    return 0;
}
