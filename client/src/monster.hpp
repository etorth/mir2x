/*
 * =====================================================================================
 *
 *       Filename: monster.hpp
 *        Created: 08/31/2015 8:26:19 PM
 *  Last Modified: 06/02/2016 17:50:08
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
#include "creature.hpp"
#include "monsterginfo.hpp"

class Monster: public Actor
{
    public:
        Monster(int, int, int);
        ~Monster();

    public:
        void Update();
        int  GenTime();

    public:
        void Draw();
        int  FrameCount();

    public:
        void UpdateCurrentState();
        void UpdateWithNewState();

    public:
        void QueryMonsterGInfo(uint32_t);

    protected:
        static MonsterGInfo m_MonsterGInfo[0X1000];

    public:
        template<typename... T> static void CreateGInfoRecord(uint32_t nMonsterID, T&&... stT)
        {
            s_MonsterGInfoMap[nMonsterID] = MonsterGInfo(nMonsterID, std::forward<T>(stT)...);
        }

        const MonsterGInfo &GetGInfoRecord(uint32_t nMonsterID)
        {
            return s_MonsterGInfoMap[nMonsterID];
        }

    protected:
        static std::unordered_map<uint32_t, MonsterGInfo> s_MonsterGInfoMap;
};
