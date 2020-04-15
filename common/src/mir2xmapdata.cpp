/*
 * =====================================================================================
 *
 *       Filename: mir2xmapdata.cpp
 *        Created: 08/31/2015 18:26:57
 *    Description: class to record data for mir2x map
 *                 this class won't define operation over the data
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

#include <vector>
#include <memory>
#include <cstdint>
#include <cstring>
#include <algorithm>

#include "mathf.hpp"
#include "sysconst.hpp"
#include "condcheck.hpp"
#include "pushstream.hpp"
#include "mir2xmapdata.hpp"

bool Mir2xMapData::Load(const char *szFullName)
{
    if(true
            && szFullName
            && std::strlen(szFullName)){

        if(auto fp = std::fopen(szFullName, "rb")){
            std::fseek(fp, 0, SEEK_END);
            auto nDataLen = std::ftell(fp);
            std::fseek(fp, 0, SEEK_SET);

            std::vector<uint8_t> stvMapData;
            stvMapData.resize(nDataLen + 1024);

            auto bReadOK = (std::fread(&(stvMapData[0]), nDataLen, 1, fp) == 1);
            std::fclose(fp);

            if(true
                    && bReadOK
                    && nDataLen >= 4
                    && Load(&(stvMapData[0]), nDataLen)){
                return true;
            }
        }
    }

    m_W = 0;
    m_H = 0;
    m_Data.clear();
    return false;
}

bool Mir2xMapData::Load(const uint8_t *pData, size_t nDataLen)
{
    if(true
            && pData
            && nDataLen >= 4){

        std::memcpy(&m_W, pData, 2); pData += 2;
        std::memcpy(&m_H, pData, 2); pData += 2;

        if(true
                && (m_W / 2 > 0) && !(m_W % 2)
                && (m_H / 2 > 0) && !(m_H % 2)){

            auto nBlockSize = (m_W / 2) * (m_H / 2);
            m_Data.resize(nBlockSize);

            if(((sizeof(m_Data[0]) * nBlockSize) + 4) == nDataLen){
                std::memcpy(&(m_Data[0]), pData, sizeof(m_Data[0]) * nBlockSize);
                return true;
            }
        }
    }

    m_W = 0;
    m_H = 0;
    m_Data.clear();
    return false;
}

bool Mir2xMapData::Save(const char *szFullName)
{
    if(Valid()){
        std::vector<uint8_t> stvByte;
        PushStream::PushByte<uint16_t>(stvByte, W());
        PushStream::PushByte<uint16_t>(stvByte, H());
        PushStream::PushByte(stvByte, Data(), Data() + DataLen());

        if(auto fp = std::fopen(szFullName, "wb")){
            auto bSaveOK = (std::fwrite(&(stvByte[0]), stvByte.size() * sizeof(stvByte[0]), 1, fp) == 1);
            std::fclose(fp);
            return bSaveOK;
        }
    }
    return false;
}

bool Mir2xMapData::Allocate(uint16_t nW, uint16_t nH)
{
    if(nW % 2 || nH % 2){ return false; }
    if(nW * nH > 0){
        m_W = nW;
        m_H = nH;

        m_Data.resize(m_W * m_H / 4);
        std::memset(&(m_Data[0]), 0, sizeof(m_Data[0]) * m_Data.size());
        return true;
    }
    return false;
}
