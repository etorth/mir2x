/*
 * =====================================================================================
 *
 *       Filename: monsterginforecord.hpp
 *        Created: 06/13/2016 23:44:15
 *  Last Modified: 03/26/2017 16:20:18
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
        uint32_t m_Data[5];

    public:
        MonsterGInfoRecord(uint32_t nMonsterID,
                uint32_t nLookID0,
                uint32_t nLookID1,
                uint32_t nLookID2,
                uint32_t nLookID3)
            : m_Data { nMonsterID, nLookID0, nLookID1, nLookID2, nLookID3 }
        {}

        MonsterGInfoRecord(uint32_t nMonsterID = 0)
            : MonsterGInfoRecord(nMonsterID, 0, 0, 0, 0)
        {}

        MonsterGInfoRecord(const uint8_t *pBuf, size_t nBufLen)
            : MonsterGInfoRecord(0)
        {
            if(nBufLen >= sizeof(m_Data)){
                std::memcpy(m_Data, pBuf, sizeof(m_Data));
            }
        }

       ~MonsterGInfoRecord() = default;

    public:
        static MonsterGInfoRecord &Null()
        {
            static MonsterGInfoRecord stNullRecord(0);
            return stNullRecord;
        }

    public:
        bool Valid() const
        {
            return m_Data[0] != 0;
        }

    public:
        uint32_t LookID(int nLookID) const
        {
            return m_Data[nLookID + 1];
        }

    public:
        const uint8_t *Data() const
        {
            return (uint8_t *)(m_Data);
        }

        size_t Size()
        {
            return sizeof(m_Data);
        }
};
