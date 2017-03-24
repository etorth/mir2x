/*
 * =====================================================================================
 *
 *       Filename: mir2xmapdata.cpp
 *        Created: 08/31/2015 18:26:57
 *  Last Modified: 03/23/2017 23:57:01
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
#include <cassert>
#include <algorithm>

#include "sysconst.hpp"
#include "mathfunc.hpp"
#include "mir2xmapdata.hpp"

int Mir2xMapData::Load(const char *szFullName)
{
    m_Data.clear();
    if(auto pFile = std::fopen(szFullName, "rb")){
        std::fseek(pFile, 0, SEEK_END);
        auto nFileSize = ftell(pFile);
        std::fseek(pFile, 0, SEEK_SET);

        std::vector<uint8_t> bufMapData;
        bufMapData.resize(nFileSize);

        void(1 + std::fread(&(bufMapData[0]), nFileSize, 1, pFile));
        std::fclose(pFile);

        // tile
        auto fnSetTile = [this](int nX, int nY, int nSize, const uint8_t *, size_t &, const uint8_t *pData, size_t &nDataOff){
            return SetTile(nX, nY, nSize, pData, nDataOff);
        };

        auto fnParseTile = [this, fnSetTile](int nX, int nY, int nSize, const uint8_t *pMark, size_t &nMarkOff, const uint8_t *pData, size_t &nDataOff){
            return ParseGrid(nX, nY, nSize, 2, pMark, nMarkOff, pData, nDataOff, fnSetTile);
        };

        // cell
        auto fnSetCell = [this](int nX, int nY, int nSize, const uint8_t *, size_t &, const uint8_t *pData, size_t &nDataOff){
            return SetCell(nX, nY, nSize, pData, nDataOff);
        };

        auto fnParseCell = [this, fnSetCell](int nX, int nY, int nSize, const uint8_t *pMark, size_t &nMarkOff, const uint8_t *pData, size_t &nDataOff){
            return ParseGrid(nX, nY, nSize, 1, pMark, nMarkOff, pData, nDataOff, fnSetCell);
        };

        // obj0
        auto fnSetObj0 = [this](int nX, int nY, int nSize, const uint8_t *pMark, size_t &nMarkOff, const uint8_t *pData, size_t &nDataOff){
            return SetObj(nX, nY, 0, nSize, pMark, nMarkOff, pData, nDataOff);
        };

        auto fnParseObj0 = [this, fnSetObj0](int nX, int nY, int nSize, const uint8_t *pMark, size_t &nMarkOff, const uint8_t *pData, size_t &nDataOff){
            return ParseGrid(nX, nY, nSize, 1, pMark, nMarkOff, pData, nDataOff, fnSetObj0);
        };

        // obj1
        auto fnSetObj1 = [this](int nX, int nY, int nSize, const uint8_t *pMark, size_t &nMarkOff, const uint8_t *pData, size_t &nDataOff){
            return SetObj(nX, nY, 1, nSize, pMark, nMarkOff, pData, nDataOff);
        };

        auto fnParseObj1 = [this, fnSetObj1](int nX, int nY, int nSize, const uint8_t *pMark, size_t &nMarkOff, const uint8_t *pData, size_t &nDataOff){
            return ParseGrid(nX, nY, nSize, 1, pMark, nMarkOff, pData, nDataOff, fnSetObj1);
        };

        auto pCurDat = &(bufMapData[0]);
        if(true
                && !LoadHead(pCurDat)
                && !LoadGrid(pCurDat, fnParseTile)
                && !LoadGrid(pCurDat, fnParseCell)
                && !LoadGrid(pCurDat, fnParseObj0)
                && !LoadGrid(pCurDat, fnParseObj1)){ return 0; }

        m_Data.clear();
    }
    return -1;
}


int Mir2xMapData::LoadHead(uint8_t * &pData)
{
    std::memcpy(&m_W, pData + 0, 2);
    std::memcpy(&m_H, pData + 2, 2);

    pData += 4;
    m_Data.resize((m_W / 2) * (m_H / 2));

    if(m_Data.empty()){ return -1; }

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
            std::memcpy(&nParam, pData, 4);
            nDataOff += 4;

            // data check here
            // saved tile parameter should have validness mark set
            // don't use g_Log since this will be linked to serveral binaries
            assert(nParam & 0X80000000);
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

int Mir2xMapData::SetCell(int nX, int nY, int nSize, const uint8_t *pData, size_t &nDataOff)
{
    if(ValidC(nX, nY) && (nSize > 0)){
        uint32_t nParam = 0;
        if(pData){
            std::memcpy(&nParam, pData, 4);
            nDataOff += 4;

            // data check here
            // saved tile parameter should have validness mark set
            // don't use g_Log since this will be linked to serveral binaries
            assert(nParam & 0X80000000);
        }

        for(int nTY = nY; nTY < nY + nSize; ++nTY){
            for(int nTX = nX; nTX < nX + nSize; ++nTX){
                if(ValidC(nTX, nTY)){
                    Cell(nTX, nTY).Param = nParam;
                }
            }
        }
        return 0;
    }
    return -1;
}

int Mir2xMapData::SetObj(int nX, int nY, int nObjIndex, int nSize, const uint8_t *, size_t &, const uint8_t *pData, size_t &nDataOff)
{
    // we define two objects are *the same* in:
    //  1. image
    //  2. animation
    //  3. blending method
    //  4. layer

    if(ValidC(nX, nY) && (nObjIndex == 0 || nObjIndex == 1) && (nSize > 0)){
        uint32_t nObjParam = 0;         // for OBJ::Param
        uint16_t nCellObjParam = 0;     // for CELL::ObjParam

        if(pData){
            std::memcpy(&nObjParam,      pData + 0, 4);  nDataOff += 4;
            std::memcpy(&nCellObjParam,  pData + 4, 2);  nDataOff += 2;

            assert(nObjParam & 0X80000000);
        }

        for(int nTY = nY; nTY < nY + nSize; ++nTY){
            for(int nTX = nX; nTX < nX + nSize; ++nTX){
                if(ValidC(nTX, nTY)){
                    if(nObjParam & 0X80000000){
                        Cell(nTX, nTY).Obj[nObjIndex].Param = nObjParam;
                        Cell(nTX, nTY).ObjParam &= (nObjIndex ? 0XFFFF0000 : 0X0000FFFF);
                        Cell(nTX, nTY).ObjParam |= (((uint32_t)(nCellObjParam)) << (nObjIndex ? 16 : 0));
                    }
                }
            }
        }
        return 0;
    }
    return -1;
}

int Mir2xMapData::LoadGrid(uint8_t * &pData, std::function<int(int, int, int, const uint8_t *, size_t &, const uint8_t *, size_t &)> fnParseGrid)
{
    uint32_t nMarkLen;
    uint32_t nDataLen;

    std::memcpy(&nMarkLen, pData + 0, 4);
    std::memcpy(&nDataLen, pData + 4, 4);

    size_t nMarkOff = 0;
    size_t nDataOff = 0;

    for(int nY= 0; nY < m_H; nY += 8){
        for(int nX= 0; nX < m_W; nX += 8){
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

int Mir2xMapData::ParseGrid(int nX, int nY, int nSize, int nFinalSize,
        const uint8_t *pMark, size_t &nMarkOff, const uint8_t *pData, size_t &nDataOff,
        std::function<int(int, int, int, const uint8_t *, size_t &, const uint8_t *, size_t &)> fnSet)
{
    // 1: current grid has data
    // 0: no
    //
    // 1: current grid is combined
    // 0: no

    if((nX >= 0) && (nY >= 0) && (nSize > 0) && (nFinalSize > 0) && (nSize >= nFinalSize) && !(nX % nFinalSize) && !(nY % nFinalSize)){
        if(ValidC(nX, nY)){
            if(PickOneBit(pMark, nMarkOff++)){
                // there is information in current grid
                if(nSize > nFinalSize){
                    // not the last level of grid, and there is information in current gird
                    if(PickOneBit(pMark, nMarkOff++)){
                        // there is data in current grid and it's combined, further parse it
                        if(ParseGrid(nX,             nY,             nSize / 2, nFinalSize, pMark, nMarkOff, pData, nDataOff, fnSet)){ return -1; }
                        if(ParseGrid(nX + nSize / 2, nY,             nSize / 2, nFinalSize, pMark, nMarkOff, pData, nDataOff, fnSet)){ return -1; }
                        if(ParseGrid(nX,             nY + nSize / 2, nSize / 2, nFinalSize, pMark, nMarkOff, pData, nDataOff, fnSet)){ return -1; }
                        if(ParseGrid(nX + nSize / 2, nY + nSize / 2, nSize / 2, nFinalSize, pMark, nMarkOff, pData, nDataOff, fnSet)){ return -1; }
                    }else{
                        // there is data in current grid and it's not combined
                        // so full-filled with same attribute
                        return fnSet(nX, nY, nSize, pMark, nMarkOff, pData, nDataOff);
                    }
                }else{
                    // last level of grid, and there is data, so fill it directly
                    return fnSet(nX, nY, nSize, pMark, nMarkOff, pData, nDataOff);
                }
            }else{
                // no data here, always unset the desc field for the whole grid
                return fnSet(nX, nY, nSize, nullptr, nMarkOff, nullptr, nDataOff);
            }
        }
        return 0;
    }

    // invalid argument
    return -1;
}

int Mir2xMapData::Save(const char *szFullName)
{
    if(Valid()){
        std::vector<bool>    stMarkV;
        std::vector<uint8_t> stDataV;
        std::vector<uint8_t> stOutV;

        // header, w then h
        {
            stOutV.push_back((uint8_t)((m_W & 0X00FF)     ));
            stOutV.push_back((uint8_t)((m_W & 0XFF00) >> 8));
            stOutV.push_back((uint8_t)((m_H & 0X00FF)     ));
            stOutV.push_back((uint8_t)((m_H & 0XFF00) >> 8));
            stOutV.push_back((uint8_t)(0));
        }

        // tile
        {
            stMarkV.clear();
            stDataV.clear();

            auto fnGridChecker = [this, nParam = (uint32_t)(0)](int nX, int nY) mutable -> int {
                if(ValidC(nX, nY) && !(nX % 2) && !(nY % 2)){
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

            auto fnAttrGridType = [this, fnGridChecker](int nX, int nY, int nSize){
                // fnGridChecker is stateful but fnAttrGridType is stateless
                // because every time when call fnAttrGridType, we use a copy of fnGridChecker at its initial state
                return GridAttrType(nX, nY, nSize, 2, fnGridChecker);
            };

            auto fnRecord = [this](int nX, int nY, std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV){
                if(ValidC(nX, nY)){
                    stMarkV.push_back(true);
                    auto pBuf = (uint8_t *)(&(Tile(nX, nY).Param));
                    stDataV.insert(stDataV.end(), pBuf, pBuf + 4);
                    return 0;
                }
                return -1;
            };

            auto fnCompressGrid = [this, fnAttrGridType, fnRecord](int nX, int nY, int nSize, std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV){
                return CompressGrid(nX, nY, nSize, 2, stMarkV, stDataV, fnAttrGridType, fnRecord);
            };

            SaveGrid(stMarkV, stDataV, fnCompressGrid);
            PushData(stMarkV, stDataV, stOutV);
        }

        // cell
        {
            stMarkV.clear();
            stDataV.clear();

            auto fnGridChecker = [this, nParam = (uint32_t)(0)](int nX, int nY) mutable -> int {
                if(ValidC(nX, nY)){
                    auto nCurrParam = Cell(nX, nY).Param;
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

            auto fnAttrGridType = [this, fnGridChecker](int nX, int nY, int nSize){
                // fnGridChecker is stateful but fnAttrGridType is stateless
                // because every time when call fnAttrGridType, we use a copy of fnGridChecker at its initial state
                return GridAttrType(nX, nY, nSize, 1, fnGridChecker);
            };

            auto fnRecord = [this](int nX, int nY, std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV){
                if(ValidC(nX, nY)){
                    stMarkV.push_back(true);
                    auto pBuf = (uint8_t *)(&(Cell(nX, nY).Param));
                    stDataV.insert(stDataV.end(), pBuf, pBuf + 4);
                    return 0;
                }
                return -1;
            };

            auto fnCompressGrid = [this, fnAttrGridType, fnRecord](int nX, int nY, int nSize, std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV){
                return CompressGrid(nX, nY, nSize, 1, stMarkV, stDataV, fnAttrGridType, fnRecord);
            };

            SaveGrid(stMarkV, stDataV, fnCompressGrid);
            PushData(stMarkV, stDataV, stOutV);
        }

        // can walk
        {
            stMarkV.clear();
            stDataV.clear();

            auto fnGridChecker = [this, nParam = (uint32_t)(0)](int nX, int nY) mutable -> int {
                if(ValidC(nX, nY)){
                    auto nCurrParam = Cell(nX, nY).Param;
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

            auto fnAttrGridType = [this, fnGridChecker](int nX, int nY, int nSize){
                // fnGridChecker is stateful but fnAttrGridType is stateless
                // because every time when call fnAttrGridType, we use a copy of fnGridChecker at its initial state
                return GridAttrType(nX, nY, nSize, 1, fnGridChecker);
            };

            auto fnRecord = [this](int nX, int nY, std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV){
                if(ValidC(nX, nY)){
                    stMarkV.push_back(true);
                    auto pBuf = (uint8_t *)(&(Cell(nX, nY).Param));
                    stDataV.insert(stDataV.end(), pBuf, pBuf + 4);
                    return 0;
                }
                return -1;
            };

            auto fnCompressGrid = [this, fnAttrGridType, fnRecord](int nX, int nY, int nSize, std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV){
                return CompressGrid(nX, nY, nSize, 1, stMarkV, stDataV, fnAttrGridType, fnRecord);
            };

            SaveGrid(stMarkV, stDataV, fnCompressGrid);
            PushData(stMarkV, stDataV, stOutV);
        }

        // obj0
        {
            stMarkV.clear();
            stDataV.clear();

            auto fnGridChecker = [this, nObjParam = (uint32_t)(0), nCellObjParam = (uint16_t)(0)](int nX, int nY) mutable -> int {
                if(ValidC(nX, nY)){
                    auto nCurrObjParam = (uint32_t)(Cell(nX, nY).Obj[0].Param);
                    if(nCurrObjParam & 0X80000000){
                        auto nCurrCellObjParam = (uint16_t)(Cell(nX, nY).ObjParam & 0X0000FFFF);
                        if(nObjParam & 0X80000000){
                            if(nObjParam == nCurrObjParam && nCellObjParam == nCurrCellObjParam){
                                // not the first time, but it's the same
                                return 2;
                            }else{
                                // not the first time, and it's different
                                return 3;
                            }
                        }else{
                            nObjParam = nCurrObjParam;
                            nCellObjParam = nCurrCellObjParam;

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

            auto fnAttrGridType = [this, fnGridChecker](int nX, int nY, int nSize){
                // fnGridChecker is stateful but fnAttrGridType is stateless
                // because every time when call fnAttrGridType, we use a copy of fnGridChecker at its initial state
                return GridAttrType(nX, nY, nSize, 1, fnGridChecker);
            };

            auto fnRecord = [this](int nX, int nY, std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV){
                if(ValidC(nX, nY)){
                    stMarkV.push_back(true);
                    // 1. record object image info
                    {
                        auto pBuf = (uint8_t *)(&(Cell(nX, nY).Obj[0].Param));
                        stDataV.insert(stDataV.end(), pBuf, pBuf + 4);
                    }

                    // 2. record blending & animation info
                    {
                        auto pBuf = (uint8_t *)(&(Cell(nX, nY).ObjParam));
                        stDataV.insert(stDataV.end(), pBuf, pBuf + 2);
                    }
                    return 0;
                }
                return -1;
            };

            auto fnCompressGrid = [this, fnAttrGridType, fnRecord](int nX, int nY, int nSize, std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV){
                return CompressGrid(nX, nY, nSize, 1, stMarkV, stDataV, fnAttrGridType, fnRecord);
            };

            SaveGrid(stMarkV, stDataV, fnCompressGrid);
            PushData(stMarkV, stDataV, stOutV);
        }

        // obj1
        {
            stMarkV.clear();
            stDataV.clear();

            auto fnGridChecker = [this, nObjParam = (uint32_t)(0), nCellObjParam = (uint16_t)(0)](int nX, int nY) mutable -> int {
                if(ValidC(nX, nY)){
                    auto nCurrObjParam = (uint32_t)(Cell(nX, nY).Obj[0].Param);
                    if(nCurrObjParam & 0X80000000){
                        auto nCurrCellObjParam = (uint16_t)((Cell(nX, nY).ObjParam & 0XFFFF0000) >> 16);
                        if(nObjParam & 0X80000000){
                            if(nObjParam == nCurrObjParam && nCellObjParam == nCurrCellObjParam){
                                // not the first time, but it's the same
                                return 2;
                            }else{
                                // not the first time, and it's different
                                return 3;
                            }
                        }else{
                            nObjParam = nCurrObjParam;
                            nCellObjParam = nCurrCellObjParam;

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

            auto fnAttrGridType = [this, fnGridChecker](int nX, int nY, int nSize){
                // fnGridChecker is stateful but fnAttrGridType is stateless
                // because every time when call fnAttrGridType, we use a copy of fnGridChecker at its initial state
                return GridAttrType(nX, nY, nSize, 1, fnGridChecker);
            };

            auto fnRecord = [this](int nX, int nY, std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV){
                if(ValidC(nX, nY)){
                    stMarkV.push_back(true);
                    // 1. record object image info
                    {
                        auto pBuf = (uint8_t *)(&(Cell(nX, nY).Obj[0].Param));
                        stDataV.insert(stDataV.end(), pBuf, pBuf + 4);
                    }

                    // 2. record blending & animation info
                    {
                        auto pBuf = (uint8_t *)(&(Cell(nX, nY).ObjParam));
                        stDataV.insert(stDataV.end(), pBuf + 2, pBuf + 4);
                    }
                    return 0;
                }
                return -1;
            };

            auto fnCompressGrid = [this, fnAttrGridType, fnRecord](int nX, int nY, int nSize, std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV){
                return CompressGrid(nX, nY, nSize, 1, stMarkV, stDataV, fnAttrGridType, fnRecord);
            };

            SaveGrid(stMarkV, stDataV, fnCompressGrid);
            PushData(stMarkV, stDataV, stOutV);
        }

        if(auto fp = std::fopen(szFullName, "wb")){
            std::fwrite(&(stOutV[0]), stOutV.size() * sizeof(stOutV[0]), 1, fp);
            std::fclose(fp);
            return 0;
        }
    }

    return -1;
}

int Mir2xMapData::SaveGrid(std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV, std::function<int(int, int, int, std::vector<bool> &, std::vector<uint8_t> &)> fnCompressGrid)
{
    stMarkV.clear();
    stDataV.clear();
    for(int nY = 0; nY < H(); nY += 8){
        for(int nX = 0; nX < W(); nX += 8){
            if(fnCompressGrid(nX, nY, 8, stMarkV, stDataV)){ return -1; }
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
//         nFinalSize :
//         fnChecker  : *stateful* checker to indicate for current state
//                    : fnChecker should always be at initialization state when call GridAttrType()
//                      returns -1 : error
//                               0 : current is empty, but we can have no-empty content detected before
//                               1 : the first time to see it
//                               2 : not the first time, but the same
//                               3 : not the first time, but with difference
int Mir2xMapData::GridAttrType(int nStartX, int nStartY, int nSize, int nFinalSize, std::function<int(int, int)> fnChecker)
{
    if(ValidC(nStartX, nStartY) && !(nStartX % nFinalSize) && !(nStartY % nFinalSize)){
        if(nSize == nFinalSize){
            switch(fnChecker(nStartX, nStartY)){
                case 0  : return  0;
                case 1  : return  1;
                case 2  : return  1;
                case 3  : return  1;
                default : return -1;
            }
        }else{
            bool bFindEmpty = false;
            bool bFindFill  = false;
            bool bFindDiff  = false;

            for(int nY = 0; nY < nSize; ++nY){
                for(int nX = 0; nX < nSize; ++nX){
                    if(ValidC(nX + nStartX, nY + nStartY) && !((nX + nStartX) % 2) && !((nY + nStartY) % 2)){
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

int Mir2xMapData::CompressGrid(int nX, int nY, int nSize, int nFinalSize, std::vector<bool> &stMarkV, std::vector<uint8_t> &stDataV, std::function<int(int, int, int)> fnAttrGridType, std::function<int(int, int, std::vector<bool> &, std::vector<uint8_t> &)> fnRecord)
{
    if(ValidC(nX, nY) && (nSize > 0) && (nFinalSize > 0) && (nSize >= nFinalSize) && !(nX % nFinalSize) && !(nY % nFinalSize)){
        switch(auto nType = fnAttrGridType(nX, nY, nSize)){
            case -1:
                {
                    return 0;
                }
            case 0:
                {
                    stMarkV.push_back(false);
                    return 0;
                }
            default:
                {
                    stMarkV.push_back(true);
                    if(nSize == nFinalSize){
                        // there is info and it's last level, so nType can only be 1
                        // end of recursion
                        return fnRecord(nX, nY, stMarkV, stDataV);
                    }else{

                        // there is info, and it's not the last level
                        if(nType == 2 || nType == 3){
                            stMarkV.push_back(true);

                            if(CompressGrid(nX            , nY            , nSize / 2, nFinalSize, stMarkV, stDataV, fnAttrGridType, fnRecord)){ return -1; }
                            if(CompressGrid(nX + nSize / 2, nY            , nSize / 2, nFinalSize, stMarkV, stDataV, fnAttrGridType, fnRecord)){ return -1; }
                            if(CompressGrid(nX            , nY + nSize / 2, nSize / 2, nFinalSize, stMarkV, stDataV, fnAttrGridType, fnRecord)){ return -1; }
                            if(CompressGrid(nX + nSize / 2, nY + nSize / 2, nSize / 2, nFinalSize, stMarkV, stDataV, fnAttrGridType, fnRecord)){ return -1; }
                        }else{
                            // nType == 1 here, filled with the same info
                            stMarkV.push_back(false);
                            return fnRecord(nX, nY, stMarkV, stDataV);
                        }
                    }

                    return 0;
                }
        }
    }

    return 0;
}

void Mir2xMapData::PushBit(const std::vector<bool> &stMarkV, std::vector<uint8_t> &stOutV)
{
    size_t nIndex = 0;
    while(nIndex < stMarkV.size()){
        uint8_t nRes = 0X00;
        for(int nBit = 0; nBit < 8; ++nBit){
            nRes = (nRes >> 1) + ((nIndex < stMarkV.size() && stMarkV[nIndex++]) ? 0X80 : 0X00);
        }
        stOutV.push_back(nRes);
    }
}

void Mir2xMapData::PushData(const std::vector<bool> &stMarkV, const std::vector<uint8_t> &stDataV, std::vector<uint8_t> &stOutV)
{
    auto nMarkLen = (uint32_t)((stMarkV.size() + 7) / 8);
    auto nDataLen = (uint32_t)((stDataV.size()));

    stOutV.push_back((uint8_t)((nMarkLen & 0X000000FF) >>  0));
    stOutV.push_back((uint8_t)((nMarkLen & 0X0000FF00) >>  8));
    stOutV.push_back((uint8_t)((nMarkLen & 0X00FF0000) >> 16));
    stOutV.push_back((uint8_t)((nMarkLen & 0XFF000000) >> 24));

    stOutV.push_back((uint8_t)((nDataLen & 0X000000FF) >>  0));
    stOutV.push_back((uint8_t)((nDataLen & 0X0000FF00) >>  8));
    stOutV.push_back((uint8_t)((nDataLen & 0X00FF0000) >> 16));
    stOutV.push_back((uint8_t)((nDataLen & 0XFF000000) >> 24));

    PushBit(stMarkV, stOutV);
    stOutV.insert(stOutV.end(), stDataV.begin(), stDataV.end());
    stOutV.push_back(0);
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
