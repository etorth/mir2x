/*
 * =====================================================================================
 *
 *       Filename: monsterginforecord.hpp
 *        Created: 06/13/2016 23:44:15
 *  Last Modified: 06/14/2016 00:21:46
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

#pragma once
#include <cstdint>

class MonsterGInfoRecord final
{
    private:
        uint32_t m_MonsterID;
        uint32_t m_LookIDV[4];
        uint32_t m_RV[4];

    public:
        MonsterGInfoRecord(uint32_t nMonsterID = 0)
            : m_MonsterID(nMonsterID)
        {}

        MonsterGInfoRecord(uint32_t, // MonsterID
                uint32_t, uint32_t, uint32_t, uint32_t,   // LookID: 0 ~ 3
                uint32_t, uint32_t, uint32_t, uint32_t);  // R:      0 ~ 3

        ~MonsterGInfoRecord()
        {}

    public:
        bool Valid() const
        {
            return m_MonsterID != 0;
        }

    public:
        uint32_t R(int) const;
        uint32_t LookID(int) const;
};
