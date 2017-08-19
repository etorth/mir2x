/*
 * =====================================================================================
 *
 *       Filename: mir2xmapdata.cpp
 *        Created: 08/31/2015 18:26:57
 *  Last Modified: 08/18/2017 18:22:14
 *
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

#include "sysconst.hpp"
#include "mathfunc.hpp"
#include "condcheck.hpp"
#include "pushstream.hpp"
#include "mir2xmapdata.hpp"

int Mir2xMapData::Load(const char *szFullName)
{
    m_Data.clear();
    if(auto pf = std::fopen(szFullName, "rb")){
        std::fseek(pf, 0, SEEK_END);
        auto nDataLen = std::ftell(pf);
        std::fseek(pf, 0, SEEK_SET);

        std::vector<uint8_t> stvMapData;
        stvMapData.resize(nDataLen + 1024);

        auto bReadOK = (std::fread(&(stvMapData[0]), nDataLen, 1, pf) == 1);
        std::fclose(pf);

        if(bReadOK){
            auto pData = &(stvMapData[0]);
            std::memcpy(&m_W, pData, 2); pData += 2;
            std::memcpy(&m_H, pData, 2); pData += 2;

            if(true
                    && (m_W > 0) && !(m_W % 2)
                    && (m_H > 0) && !(m_H % 2)){
                m_Data.resize((m_W / 2) * (m_H / 2));
            }else{
                return -1;
            }

            // [1:0] : W
            // [3:2] : H
            // [...] : BLOCK[...]
            condcheck(((sizeof(m_Data[0]) * m_W * m_H / 4) + 4) == (size_t)(nDataLen));
            std::memcpy(&(m_Data[0]), pData, m_Data.size() * sizeof(m_Data[0]));
            return 0;
        }
    }

    m_Data.clear();
    return -1;
}

int Mir2xMapData::Save(const char *szFullName)
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
    return -1;
}

bool Mir2xMapData::Allocate(uint16_t nW, uint16_t nH)
{
    if(nW % 2 || nH % 2){ return false; }
    if(nW * nH){
        m_W = nW;
        m_H = nH;

        m_Data.resize(m_W * m_H / 4);
        std::memset(&(m_Data[0]), 0, sizeof(m_Data[0]) * m_Data.size());
        return true;
    }
    return false;
}
