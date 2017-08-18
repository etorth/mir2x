/*
 * =====================================================================================
 *
 *       Filename: mir2xmapdata.cpp
 *        Created: 08/31/2015 18:26:57
 *  Last Modified: 08/18/2017 16:14:05
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
            auto fnParseTile = [this](int nX, int nY, int nSize, const uint8_t *pMark, size_t &nMarkOff, const uint8_t *pData, size_t &nDataOff)
            {
                auto fnSetTile = [this](int nX, int nY, int nSize, const uint8_t *pData, size_t &nDataOff)
                {
                    return SetTile(nX, nY, nSize, pData, nDataOff);
                };
                return ParseGrid(nX, nY, nSize, 2, pMark, nMarkOff, pData, nDataOff, fnSetTile);
            };

            auto fnParseLight = [this](int nX, int nY, int nSize, const uint8_t *pMark, size_t &nMarkOff, const uint8_t *pData, size_t &nDataOff)
            {
                auto fnSetLight = [this](int nX, int nY, int nSize, const uint8_t *pData, size_t &nDataOff)
                {
                    return SetLight(nX, nY, nSize, pData, nDataOff);
                };
                return ParseGrid(nX, nY, nSize, 1, pMark, nMarkOff, pData, nDataOff, fnSetLight);
            };

            auto fnParseLand = [this](int nX, int nY, int nSize, const uint8_t *pMark, size_t &nMarkOff, const uint8_t *pData, size_t &nDataOff)
            {
                auto fnSetLand = [this](int nX, int nY, int nSize, const uint8_t *pData, size_t &nDataOff)
                {
                    return SetLand(nX, nY, nSize, pData, nDataOff);
                };
                return ParseGrid(nX, nY, nSize, 1, pMark, nMarkOff, pData, nDataOff, fnSetLand);
            };

            auto fnParseObj0 = [this](int nX, int nY, int nSize, const uint8_t *pMark, size_t &nMarkOff, const uint8_t *pData, size_t &nDataOff)
            {
                auto fnSetObj0 = [this](int nX, int nY, int nSize, const uint8_t *pData, size_t &nDataOff)
                {
                    return SetObj(nX, nY, 0, nSize, pData, nDataOff);
                };
                return ParseGrid(nX, nY, nSize, 1, pMark, nMarkOff, pData, nDataOff, fnSetObj0);
            };

            auto fnParseObj1 = [this](int nX, int nY, int nSize, const uint8_t *pMark, size_t &nMarkOff, const uint8_t *pData, size_t &nDataOff)
            {
                auto fnSetObj1 = [this](int nX, int nY, int nSize, const uint8_t *pData, size_t &nDataOff)
                {
                    return SetObj(nX, nY, 1, nSize, pData, nDataOff);
                };
                return ParseGrid(nX, nY, nSize, 1, pMark, nMarkOff, pData, nDataOff, fnSetObj1);
            };

            auto pCurDat = &(stvMapData[0]);
            if(true
                    && !LoadHead(pCurDat)
                    && !LoadGrid(pCurDat, fnParseTile )
                    && !LoadGrid(pCurDat, fnParseLight)
                    && !LoadGrid(pCurDat, fnParseLand )
                    && !LoadGrid(pCurDat, fnParseObj0 )
                    && !LoadGrid(pCurDat, fnParseObj1 )){ return 0; }
        }
    }

    m_Data.clear();
    return -1;
}

int Mir2xMapData::LoadHead(uint8_t * &pData)
{
    std::memcpy(&m_W, pData, 2); pData += 2;
    std::memcpy(&m_H, pData, 2); pData += 2;

    if(true
            && (m_W > 0) && !(m_W % 2)
            && (m_H > 0) && !(m_H % 2)){
        m_Data.resize((m_W / 2) * (m_H / 2));
    }else{
        return -1;
    }

    if(pData[0] != 0){
        return -1;
    }else{
        pData++;
        return 0;
    }
}

int Mir2xMapData::SetTile(int nX, int nY, int nSize, const uint8_t *pData, size_t &nDataOff)
{
    if(ValidC(nX, nY) && (nSize > 0)){
        uint32_t nParam = 0;
        if(pData){
            std::memcpy(&nParam, pData + nDataOff, 4);
            nDataOff += 4;

            // data check here
            // saved tile parameter should have validness mark set
            // don't use g_Log since this will be linked to serveral binaries
            condcheck(nParam & 0X80000000);
        }

        for(int nTY = nY; nTY < nY + nSize; ++nTY){
            for(int nTX = nX; nTX < nX + nSize; ++nTX){
                if(ValidC(nTX, nTY) && (!(nTX % 2)) && (!(nTY % 2))){
                    Tile(nTX, nTY).Param = nParam;
                }
            }
        }
        return 0;
    }
    return -1;
}

int Mir2xMapData::SetLand(int nX, int nY, int nSize, const uint8_t *pData, size_t &nDataOff)
{
    if(ValidC(nX, nY) && (nSize > 0)){
        uint8_t nLandByte = 0;
        if(pData){
            nLandByte = pData[nDataOff++];

            // land attribute doesn't have ``valid" filed
            // every cell must have can go through field and land type can be none

            // land type is additional feature
            // to play sound effect when creature run on it
        }

        for(int nTY = nY; nTY < nY + nSize; ++nTY){
            for(int nTX = nX; nTX < nX + nSize; ++nTX){
                if(ValidC(nTX, nTY)){
                    Cell(nTX, nTY).Param |= (((uint32_t)(nLandByte)) << 16);
                }
            }
        }
        return 0;
    }
    return -1;
}

int Mir2xMapData::SetLight(int nX, int nY, int nSize, const uint8_t *pData, size_t &nDataOff)
{
    if(ValidC(nX, nY) && (nSize > 0)){
        uint8_t nLightByte = 0;
        if(pData){
            nLightByte = pData[nDataOff++];

            // data check here
            // saved tile parameter should have validness mark set
            // don't use g_Log since this will be linked to serveral binaries
            condcheck(nLightByte & 0X80);
        }

        for(int nTY = nY; nTY < nY + nSize; ++nTY){
            for(int nTX = nX; nTX < nX + nSize; ++nTX){
                if(ValidC(nTX, nTY)){
                    Cell(nTX, nTY).Param |= (((uint32_t)(nLightByte)) << 8);
                }
            }
        }
        return 0;
    }
    return -1;
}

int Mir2xMapData::SetObj(int nX, int nY, int nObjIndex, int nSize, const uint8_t *pData, size_t &nDataOff)
{
    // we define two objects are *the same* in:
    //  1. image
    //  2. animation
    //  3. blending method
    //  4. layer

    if(ValidC(nX, nY) && ((nObjIndex == 0) || (nObjIndex == 1)) && (nSize > 0)){

        // when one object is valid
        // its attributes are arraged in 5 bytes

        uint8_t nByte0 = 0;
        uint8_t nByte1 = 0;
        uint8_t nByte2 = 0;
        uint8_t nByte3 = 0;
        uint8_t nByte4 = 0;

        if(pData){
            nByte0 = pData[nDataOff++];
            nByte1 = pData[nDataOff++];
            nByte2 = pData[nDataOff++];
            nByte3 = pData[nDataOff++];
            nByte4 = pData[nDataOff++];

            // if we try to load an object
            // then current object should be ``valid"
            // if to clean an object we should pass nullptr to pData
            condcheck(nByte4 & 0X80);
        }

        for(int nTY = nY; nTY < nY + nSize; ++nTY){
            for(int nTX = nX; nTX < nX + nSize; ++nTX){
                if(ValidC(nTX, nTY)){
                    Cell(nTX, nTY).ObjParam             |= (((uint32_t)(nByte3       )) << (nObjIndex ? 24 : 8));
                    Cell(nTX, nTY).ObjParam             |= (((uint32_t)(nByte4 & 0X03)) << (nObjIndex ? 22 : 6));
                    Cell(nTX, nTY).Obj[nObjIndex].Param |= (((uint32_t)(nByte4 & 0X80)) << 23);
                    Cell(nTX, nTY).Obj[nObjIndex].Param |= (((uint32_t)(nByte2       )) << 16);
                    Cell(nTX, nTY).Obj[nObjIndex].Param |= (((uint32_t)(nByte1       )) <<  8);
                    Cell(nTX, nTY).Obj[nObjIndex].Param |= (((uint32_t)(nByte0       )) <<  0);
                }
            }
        }
        return 0;
    }
    return -1;
}

int Mir2xMapData::LoadGrid(uint8_t * &pData, std::function<int(int, int, int, const uint8_t *, size_t &, const uint8_t *, size_t &)> fnParseGrid)
{
    uint32_t nMarkLen = 0;
    uint32_t nDataLen = 0;

    std::memcpy(&nMarkLen, pData + 0, 4);
    std::memcpy(&nDataLen, pData + 4, 4);

    size_t nMarkOff = 0;
    size_t nDataOff = 0;

    for(int nY= 0; nY < H(); nY += 8){
        for(int nX= 0; nX < W(); nX += 8){
            if(fnParseGrid(nX, nY, 8, pData + 8, nMarkOff, pData + 8 + nMarkLen, nDataOff)){ return -1; }
        }
    }

    pData += (8 + nMarkLen + nDataLen);
    if(pData[0] || ((size_t)(nMarkLen) != (nMarkOff + 7) / 8) || ((size_t)(nDataLen) != nDataOff)){
        return -1;
    }else{
        pData++;
        return 0;
    }
}

int Mir2xMapData::ParseGrid(
        int nX,
        int nY,

        size_t nSize,
        size_t nEndSize,

        const uint8_t *pMark, size_t &nMarkOff,
        const uint8_t *pData, size_t &nDataOff,
        std::function<int(int, int, int, const uint8_t *, size_t &)> fnSet)
{
    // 1: current grid has data
    // 0: no
    //
    // 1: current grid is combined
    // 0: no

    if((nX >= 0) && (nY >= 0) && (nSize > 0) && (nEndSize > 0) && (nSize >= nEndSize) && (!(nX % nEndSize)) && (!(nY % nEndSize))){
        if(ValidC(nX, nY)){
            if(PushStream::PickOneBit(pMark, nMarkOff++)){
                // there is information in current grid
                if(nSize > nEndSize){
                    // not the last level of grid, and there is information in current gird
                    if(PushStream::PickOneBit(pMark, nMarkOff++)){
                        // there is data in current grid and it's combined, further parse it
                        if(ParseGrid(nX,             nY,             nSize / 2, nEndSize, pMark, nMarkOff, pData, nDataOff, fnSet)){ return -1; }
                        if(ParseGrid(nX + nSize / 2, nY,             nSize / 2, nEndSize, pMark, nMarkOff, pData, nDataOff, fnSet)){ return -1; }
                        if(ParseGrid(nX,             nY + nSize / 2, nSize / 2, nEndSize, pMark, nMarkOff, pData, nDataOff, fnSet)){ return -1; }
                        if(ParseGrid(nX + nSize / 2, nY + nSize / 2, nSize / 2, nEndSize, pMark, nMarkOff, pData, nDataOff, fnSet)){ return -1; }
                    }else{
                        // there is data in current grid and it's not combined
                        // so full-filled with same attribute
                        return fnSet(nX, nY, nSize, pData, nDataOff);
                    }
                }else{
                    // last level of grid, and there is data, so fill it directly
                    return fnSet(nX, nY, nSize, pData, nDataOff);
                }
            }else{
                // no data here, always unset the desc field for the whole grid
                size_t nDumbOff = 0;
                return fnSet(nX, nY, nSize, nullptr, nDumbOff);
            }
        }
        return 0;
    }

    // invalid argument
    return -1;
}

int Mir2xMapData::SaveHead(std::vector<uint8_t> &stvByte)
{
    if(Valid()){
        // header, w then h
        PushStream::PushByte<uint16_t>(stvByte, W());
        PushStream::PushByte<uint16_t>(stvByte, H());
        PushStream::PushByte<uint8_t >(stvByte,   0);
        return 0;
    }
    return -1;
}

int Mir2xMapData::SaveTile(std::vector<uint8_t> &stvByte)
{
    if(Valid()){
        auto fnGridChecker = [this, nParam = (uint32_t)(0)](int nX, int nY) mutable -> int
        {
            if(ValidC(nX, nY) && (!(nX % 2)) && (!(nY % 2))){
                auto nCurrParam = Tile(nX, nY).Param;
                if(nCurrParam & 0X80000000){
                    if(nParam & 0X80000000){
                        if(nParam == nCurrParam){
                            // not the first time, but it's the same
                            return 2;
                        }else{
                            // not the first time, and it's different
                            return 3;
                        }
                    }else{
                        nParam = nCurrParam;

                        // first time to see it
                        return 1;
                    }
                }else{
                    // empty
                    return 0;
                }
            }
            return -1;
        };

        auto fnAttrGridType = [this, fnGridChecker](int nX, int nY, int nSize) -> int
        {
            return GridAttrType(nX, nY, nSize, 2, fnGridChecker);
        };

        auto fnRecord = [this](int nX, int nY, std::vector<bool> &, std::vector<uint8_t> &stvData) -> int
        {
            if(ValidC(nX, nY)){
                PushStream::PushByte<uint32_t>(stvData, Tile(nX, nY).Param);
                return 0;
            }
            return -1;
        };

        auto fnCompressGrid = [this, fnAttrGridType, fnRecord](int nX, int nY, int nSize, std::vector<bool> &stvMark, std::vector<uint8_t> &stvData) -> int
        {
            return CompressGrid(nX, nY, nSize, 2, stvMark, stvData, fnAttrGridType, fnRecord);
        };

        std::vector<bool>    stvMark;
        std::vector<uint8_t> stvData;

        SaveGrid(stvMark, stvData, fnCompressGrid);
        PushData(stvByte, stvMark, stvData);
        return 0;
    }
    return -1;
}

int Mir2xMapData::SaveLand(std::vector<uint8_t> &stvByte)
{
    if(Valid()){
        // for land information any value of u8 is vaid
        // then we need another flag variable to indicate initialization
        auto fnGridChecker = [this, nLandByte = (uint8_t)(0), bInit = false](int nX, int nY) mutable -> int
        {
            if(ValidC(nX, nY)){
                auto nCurrLandByte = Cell(nX, nY).LandByte();
                if(1){
                    // land info can't be empty
                    // any value should be ``valid" for a grid

                    if(bInit){
                        if(nLandByte == nCurrLandByte){
                            // not the first time, but it's the same
                            return 2;
                        }else{
                            // not the first time, and it's different
                            return 3;
                        }
                    }else{
                        bInit = true;
                        nLandByte = nCurrLandByte;

                        // first time to see it
                        return 1;
                    }
                }else{
                    // empty
                    return 0;
                }
            }
            return -1;
        };

        auto fnAttrGridType = [this, fnGridChecker](int nX, int nY, int nSize) -> int
        {
            return GridAttrType(nX, nY, nSize, 1, fnGridChecker);
        };

        auto fnRecord = [this](int nX, int nY, std::vector<bool> &, std::vector<uint8_t> &stvData) -> int
        {
            if(ValidC(nX, nY)){
                PushStream::PushByte<uint8_t>(stvData, Cell(nX, nY).LandByte());
                return 0;
            }
            return -1;
        };

        auto fnCompressGrid = [this, fnAttrGridType, fnRecord](int nX, int nY, int nSize, std::vector<bool> &stvMark, std::vector<uint8_t> &stvData) -> int
        {
            return CompressGrid(nX, nY, nSize, 1, stvMark, stvData, fnAttrGridType, fnRecord);
        };

        std::vector<bool>    stvMark;
        std::vector<uint8_t> stvData;

        SaveGrid(stvMark, stvData, fnCompressGrid);
        PushData(stvByte, stvMark, stvData);
        return 0;
    }
    return -1;
}

int Mir2xMapData::SaveLight(std::vector<uint8_t> &stvByte)
{
    if(Valid()){
        auto fnGridChecker = [this, nLightByte = (uint8_t)(0)](int nX, int nY) mutable -> int
        {
            if(ValidC(nX, nY)){
                auto nCurrLightByte = Cell(nX, nY).LightByte();
                if(nCurrLightByte & 0X80){
                    if(nLightByte & 0X80){
                        if(nLightByte == nCurrLightByte){
                            // not the first time, but it's the same
                            return 2;
                        }else{
                            // not the first time, and it's different
                            return 3;
                        }
                    }else{
                        nLightByte = nCurrLightByte;

                        // first time to see it
                        return 1;
                    }
                }else{
                    // empty
                    return 0;
                }
            }
            return -1;
        };

        auto fnAttrGridType = [this, fnGridChecker](int nX, int nY, int nSize) -> int
        {
            // fnGridChecker is stateful but fnAttrGridType is stateless
            // because every time when call fnAttrGridType, we use a copy of fnGridChecker at its initial state
            return GridAttrType(nX, nY, nSize, 1, fnGridChecker);
        };

        auto fnRecord = [this](int nX, int nY, std::vector<bool> &, std::vector<uint8_t> &stvData) -> int
        {
            if(ValidC(nX, nY)){
                PushStream::PushByte<uint8_t>(stvData, Cell(nX, nY).LightByte());
                return 0;
            }
            return -1;
        };

        auto fnCompressGrid = [this, fnAttrGridType, fnRecord](int nX, int nY, int nSize, std::vector<bool> &stvMark, std::vector<uint8_t> &stvData) -> int
        {
            return CompressGrid(nX, nY, nSize, 1, stvMark, stvData, fnAttrGridType, fnRecord);
        };

        std::vector<bool>    stvMark;
        std::vector<uint8_t> stvData;

        SaveGrid(stvMark, stvData, fnCompressGrid);
        PushData(stvByte, stvMark, stvData);
        return 0;
    }
    return -1;
}

int Mir2xMapData::SaveObj(std::vector<uint8_t> &stvByte, int nIndex)
{
    if(Valid() && ((nIndex == 0) || (nIndex == 1))){
        auto fnGridChecker = [this, nIndex, stArray = std::array<uint8_t, 5>{0, 0, 0, 0, 0}](int nX, int nY) mutable -> int
        {
            if(ValidC(nX, nY) && ((nIndex == 0) || (nIndex == 1))){
                auto stCurrArray = Cell(nX, nY).ObjectArray(nIndex);
                if(stCurrArray[4] & 0X80){
                    if(stArray[4] & 0X80){
                        if(stCurrArray == stArray){
                            // not the first time, but it's the same
                            return 2;
                        }else{
                            // not the first time, and it's different
                            return 3;
                        }
                    }else{
                        stArray = stCurrArray;

                        // first time to see it
                        return 1;
                    }
                }else{
                    // empty
                    return 0;
                }
            }
            return -1;
        };

        auto fnAttrGridType = [this, fnGridChecker](int nX, int nY, int nSize)
        {
            return GridAttrType(nX, nY, nSize, 1, fnGridChecker);
        };

        auto fnRecord = [this, nIndex](int nX, int nY, std::vector<bool> &, std::vector<uint8_t> &stvData)
        {
            if(ValidC(nX, nY)){
                auto stArray = Cell(nX, nY).ObjectArray(nIndex);
                PushStream::PushByte(stvData, stArray.begin(), stArray.end());
                return 0;
            }
            return -1;
        };

        auto fnCompressGrid = [this, fnAttrGridType, fnRecord](int nX, int nY, int nSize, std::vector<bool> &stvMark, std::vector<uint8_t> &stvData)
        {
            return CompressGrid(nX, nY, nSize, 1, stvMark, stvData, fnAttrGridType, fnRecord);
        };

        std::vector<bool>    stvMark;
        std::vector<uint8_t> stvData;

        SaveGrid(stvMark, stvData, fnCompressGrid);
        PushData(stvByte, stvMark, stvData);
        return 0;
    }
    return -1;
}

int Mir2xMapData::Save(const char *szFullName)
{
    if(Valid()){
        std::vector<uint8_t> stvByte;
        if(true
                && !SaveHead (stvByte)
                && !SaveTile (stvByte)
                && !SaveLand (stvByte)
                && !SaveLight(stvByte)
                && !SaveObj  (stvByte, 0)
                && !SaveObj  (stvByte, 1)){

            if(auto fp = std::fopen(szFullName, "wb")){
                auto bSaveOK = (std::fwrite(&(stvByte[0]), stvByte.size() * sizeof(stvByte[0]), 1, fp) == 1);
                std::fclose(fp);
                return bSaveOK;
            }
        }
    }
    return -1;
}

int Mir2xMapData::SaveGrid(std::vector<bool> &stvMark, std::vector<uint8_t> &stvData, std::function<int(int, int, int, std::vector<bool> &, std::vector<uint8_t> &)> fnCompressGrid)
{
    stvMark.clear();
    stvData.clear();
    for(int nY = 0; nY < H(); nY += 8){
        for(int nX = 0; nX < W(); nX += 8){
            if(fnCompressGrid(nX, nY, 8, stvMark, stvData)){ return -1; }
        }
    }
    return 0;
}

// return -1 : error
//         0 : empty
//         1 : full-filled and unified
//         2 : full-filled but with diverse
//         3 : partically filled
// parameter :
//         nStartX    : 
//         nStartY    :
//         nSize      :
//         nEndSize :
//         fnChecker  : *stateful* checker to indicate for current state
//                    : fnChecker should always be at initialization state when call GridAttrType()
//                      returns -1 : error
//                               0 : current is empty, but we can have no-empty content detected before
//                               1 : the first time to see it
//                               2 : not the first time, but the same
//                               3 : not the first time, but with difference
int Mir2xMapData::GridAttrType(int nStartX, int nStartY, int nSize, int nEndSize, std::function<int(int, int)> fnChecker)
{
    if(ValidC(nStartX, nStartY) && !(nStartX % nEndSize) && !(nStartY % nEndSize)){
        if(nSize == nEndSize){
            switch(fnChecker(nStartX, nStartY)){
                case 0  : return  0;
                case 1  : return  1;
                case 2  : return -1;
                case 3  : return -1;
                default : return -1;
            }
        }else{
            bool bFindEmpty = false;
            bool bFindFill  = false;
            bool bFindDiff  = false;

            for(int nY = 0; nY < nSize; ++nY){
                for(int nX = 0; nX < nSize; ++nX){
                    if(ValidC(nX + nStartX, nY + nStartY) && !((nX + nStartX) % nEndSize) && !((nY + nStartY) % nEndSize)){
                        switch(fnChecker(nX + nStartX, nY + nStartY)){
                            case 0: // empty
                                {
                                    bFindEmpty = true;
                                    break;
                                }
                            case 1: // the first time to see it
                                {
                                    bFindFill = true;
                                    break;
                                }
                            case 2: // not the first time, but it's the same
                                {
                                    bFindFill = true;
                                    break;
                                }
                            case 3: // not the first time, and it's different!
                                {
                                    bFindFill = true;
                                    bFindDiff = true;
                                    break;
                                }
                            default:
                                {
                                    return -1;
                                }
                        }
                    }
                }
            }

            if(bFindFill == false){
                // no information at all
                return 0;
            }else{
                // do have information
                if(bFindEmpty == false){
                    // all filled
                    if(bFindDiff == false){
                        // all filled and no difference exists
                        return 1;
                    }else{
                        // all filled and there is difference
                        return 2;
                    }
                }else{
                    // there are filled and empty, combined
                    return 3;
                }
            }
        }
    }

    return -1;
}

int Mir2xMapData::CompressGrid(int nX, int nY, int nSize, int nEndSize, std::vector<bool> &stvMark, std::vector<uint8_t> &stvData, std::function<int(int, int, int)> fnAttrGridType, std::function<int(int, int, std::vector<bool> &, std::vector<uint8_t> &)> fnRecord)
{
    if(ValidC(nX, nY) && (nSize > 0) && (nEndSize > 0) && (nSize >= nEndSize) && !(nX % nEndSize) && !(nY % nEndSize)){
        switch(auto nType = fnAttrGridType(nX, nY, nSize)){
            case -1:
                {
                    return -1;
                }
            case 0:
                {
                    stvMark.push_back(false);
                    return 0;
                }
            default:
                {
                    stvMark.push_back(true);
                    if(nSize == nEndSize){
                        // there is info and it's last level, so nType can only be 1
                        // end of recursion
                        return fnRecord(nX, nY, stvMark, stvData);
                    }else{

                        // there is info, and it's not the last level
                        if(nType == 2 || nType == 3){
                            stvMark.push_back(true);

                            if(CompressGrid(nX            , nY            , nSize / 2, nEndSize, stvMark, stvData, fnAttrGridType, fnRecord)){ return -1; }
                            if(CompressGrid(nX + nSize / 2, nY            , nSize / 2, nEndSize, stvMark, stvData, fnAttrGridType, fnRecord)){ return -1; }
                            if(CompressGrid(nX            , nY + nSize / 2, nSize / 2, nEndSize, stvMark, stvData, fnAttrGridType, fnRecord)){ return -1; }
                            if(CompressGrid(nX + nSize / 2, nY + nSize / 2, nSize / 2, nEndSize, stvMark, stvData, fnAttrGridType, fnRecord)){ return -1; }
                        }else{
                            // nType == 1 here, filled with the same info
                            stvMark.push_back(false);
                            return fnRecord(nX, nY, stvMark, stvData);
                        }
                    }

                    return 0;
                }
        }
    }

    return 0;
}

void Mir2xMapData::PushData(std::vector<uint8_t> &stvByte, const std::vector<bool> &stvMark, const std::vector<uint8_t> &stvData)
{
    auto nMarkLen = (uint32_t)((stvMark.size() + 7) / 8);
    auto nDataLen = (uint32_t)((stvData.size()));

    PushStream::PushByte<uint32_t>(stvByte, nMarkLen);
    PushStream::PushByte<uint32_t>(stvByte, nDataLen);

    PushStream::PushBit(stvByte, stvMark);
    stvByte.insert(stvByte.end(), stvData.begin(), stvData.end());
    stvByte.push_back(0);
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
