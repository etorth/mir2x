/*
 * =====================================================================================
 *
 *       Filename: memorychunkbase.hpp
 *        Created: 05/12/2016 23:21:50
 *  Last Modified: 05/12/2016 23:36:50
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

class MemoryChunkBase
{
    private:
        uint8_t *m_Data;
        size_t   m_Param;

    public:
        MemoryChunk(uint8_t *pData, size_t nParam)
            : m_Data(pData)
            , m_Param(nParam)
        {}

        MemoryChunk(MemoryChunk &&rstMC)
        {
            std::swap(m_Data, rstMC.m_Data);
            std::swap(m_Param, rstMC.m_Param);
        }

        MemoryChunk(const MemoryChunk &) = delete;

        virtual ~MemoryChunkBase() = default;

    public:
        operator uint8_t* ()
        {
            return Data();
        }

    public:
        uint8_t *Data()
        {
            return m_Data;
        }

        size_t Param()
        {
            return m_Param;
        }
};
