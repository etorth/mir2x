/*
 * =====================================================================================
 *
 *       Filename: monster.hpp
 *        Created: 08/31/2015 08:26:19 PM
 *  Last Modified: 04/25/2017 00:21:05
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
        const uint32_t m_MonsterID;

    protected:
        uint32_t m_LookIDN;

    protected:
        Monster(uint32_t nUID, uint32_t nMonsterID, ProcessRun *pRun)
            : Creature(nUID, pRun)
            , m_MonsterID(nMonsterID)
            , m_LookIDN(0)
        {
            assert(nUID);
            assert(nMonsterID);
            assert(pRun);
        }

       ~Monster() = default;

    public:
       static Monster *Create(uint32_t, uint32_t, ProcessRun *, const ActionNode &);

    public:
        int Type()
        {
            return CREATURE_MONSTER;
        }

    public:
        bool Draw(int, int);
        bool Update();

    protected:
        virtual int32_t GfxID(int, int);

    public:
        uint32_t MonsterID() const
        {
            return m_MonsterID;
        }

    protected:
        bool Location(int *, int *);

    public:
        // this is the only function we have to take care of MOTION_NONE
        // for all rest part we always assume current object is in valid state
        bool ParseNewState (const  StateNode &, bool);
        bool ParseNewAction(const ActionNode &, bool);

    public:
        bool ActionValid(const ActionNode &);
        bool MotionValid(const MotionNode &);

    public:
        bool OnReportState();
        bool OnReportAction(int, int, int, int, int, int);

    public:
        bool ValidG()
        {
            return GetGInfoRecord(m_MonsterID).Valid(m_LookIDN);
        }

        uint32_t LookID()
        {
            return GetGInfoRecord(m_MonsterID).LookID(m_LookIDN);
        }

    public:
        size_t MotionFrameCount();

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
