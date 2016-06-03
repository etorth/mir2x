/*
 * =====================================================================================
 *
 *       Filename: monster.hpp
 *        Created: 08/31/2015 08:26:19 PM
 *  Last Modified: 06/03/2016 12:08:08
 *
 *    Description: monster class for client, I am concerned about whether this class
 *                 will be messed up with class monster for server side
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

#pragma once
#include "game.hpp"
#include "creature.hpp"
#include "protocoldef.hpp"
#include "monsterginfo.hpp"
#include "clientmessage.hpp"

class Monster: public Creature
{
    protected:
        uint32_t m_MonsterID;       // monster id
        uint32_t m_LookIDN;         // look effect index 0 ~ 3

    public:
        Monster(uint32_t, uint32_t, uint32_t);
        ~Monster();

    public:
        int Type()
        {
            return CREATURE_MONSTER;
        }

    public:
        void Draw();
        void Update();

    public:
        uint32_t LookID()
        {
            return GetGInfoRecord(m_MonsterID).LookID(m_LookIDN);
        }

    public:
        size_t FrameCount()
        {
            return GetGInfoRecord(m_MonsterID).FrameCount(m_LookIDN, m_State, m_Direction);
        }

    public:
        template<typename... T> static void ResetGInfoRecord(uint32_t nMonsterID, int nLookIDN, T&&... stT)
        {
            GetGInfoRecord(nMonsterID).ResetLookID(nLookIDN, std::forward<T>(stT)...);
        }

        static MonsterGInfo &GetGInfoRecord(uint32_t nMonsterID)
        {
            auto pRecord = s_MonsterGInfoMap.find(nMonsterID);
            if(pRecord != s_MonsterGInfoMap.end()){
                return pRecord->second;
            }

            // 1. we need to create a record
            s_MonsterGInfoMap.emplace(nMonsterID, nMonsterID);
            return s_MonsterGInfoMap[nMonsterID];
        }

        static void QueryGInfoRecord(uint32_t nMonsterID, uint32_t nLookIDN)
        {
            CMQueryMonsterGInfo stCMQMGI;

            stCMQMGI.MonsterID = nMonsterID;
            stCMQMGI.LookIDN   = nLookIDN;

            extern Game *g_Game;
            g_Game->Send(CM_QUERYMONSTERGINFO, stCMQMGI);
        }

    protected:
        static std::unordered_map<uint32_t, MonsterGInfo> s_MonsterGInfoMap;
};
